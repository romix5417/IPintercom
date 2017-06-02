#include <stdio.h>
#include <lmlog.h>

#define IPinterCom_VER "v0.1"

#define TCP_MODE

#define SUCCESS  0
#define ERROR   -1

int Sock;

/* Check if IPinterCom is already running: /var/run/IPinterCom.pid */
int pid_file_check_not_exist()
{
    FILE *pid_file;

    pid_file = fopen("/var/run/IPinterCom.pid", "r");
    if (pid_file != NULL)
    {
        LMLOG(LCRIT, "Check no other instance of IPinterCom-d is running. If no instance is running, remove /var/run/IPinterCom.pid");
        fclose(pid_file);
        return (BAD);
    }

    return (GOOD);
}

void exit_cleanup()
{
    close(Sock);
}

/* Creates the PID file of the process */
int pid_file_create()
{
    FILE *pid_file;
    int pid = getpid();

    pid_file = fopen("/var/run/IPinterCom.pid", "w");
    if (pid_file == NULL){
        LMLOG(LCRIT, "pid_file_create: Error creating PID file: %s",strerror(errno));
        return (BAD);
    }
    fprintf(pid_file, "%d\n",pid);
    fclose(pid_file);

    LMLOG(LDBG_1, "PID file created: /var/run/IPinterCom.pid -> %d",pid);

    return (GOOD);
}

struct option long_option[] =
{
    {"help",    0, NULL, 'h'},
    {"num",     1, NULL, 'n'},
    {"debug",   1, NULL, 'd'},
	{"server"   1, NULL, 's'},
    {"transport 1, NULL, 't'},
    {"port"     1, NULL, 'p'},
    {NULL, 0, NULL, 0},
};

void signal_handler(int sig)
{
	switch (sig) {
        case SIGHUP:
            /* TODO: SIGHUP should trigger reloading the configuration file */
            LMLOG(LDBG_1, "Received SIGHUP signal.");
            break;
        case SIGTERM:
            /* SIGTERM is the default signal sent by 'kill'. Exit cleanly */
            LMLOG(LDBG_1, "Received SIGTERM signal. Cleaning up...");
            exit_cleanup();
            break;
        case SIGINT:
            /* SIGINT is sent by pressing Ctrl-C. Exit cleanly */
            LMLOG(LDBG_1, "Terminal interrupt. Cleaning up...");
            exit_cleanup();
            break;
        default:
            LMLOG(LDBG_1,"Unhandled signal (%d)", sig);
            exit(EXIT_FAILURE);
    }
}

static void setup_signal_handlers()
{
    signal(SIGHUP,  signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGINT,  signal_handler);
    signal(SIGQUIT, signal_handler);
#ifdef TCP_MODE
    signal(SIGPIPE, SIG_IGN);
#endif
}

void help(void)
{
	printf("Usage: meshcom [OPTION]... \n"
		"--help         help\n"
		"-n, --num      set device num\n"
		"-d, --debug    set debug level\n"
		"-s, --server    set the node mode,the value is 0 or 1\n"
		"-t, --tranport set the transport mode, the value is 'tcp' or 'udp'\n"
		"-p, --port     set port num\n ");
	printf("\n");
	printf("Tip #1 the program use to mesh station device,\n"
		"usage: meshcom -n 100 -d 3\n");
}

/*
 *  Check for superuser privileges
 */
int check_capabilities()
{
    struct __user_cap_header_struct cap_header;
    struct __user_cap_data_struct cap_data;

    cap_header.pid = getpid();
    cap_header.version = _LINUX_CAPABILITY_VERSION;

    /* Check if IPinterCom is already running: /var/run/intercom.pid */

    if (capget(&cap_header, &cap_data) < 0)
    {
        LMLOG(LCRIT, "Could not retrieve capabilities");
        return BAD;
    }

    LMLOG(LINF, "Rights: Effective [%u] Permitted  [%u]", cap_data.effective, cap_data.permitted);

    /* check for capabilities */
    if(  (cap_data.effective & CAP_TO_MASK(CAP_NET_ADMIN)) && (cap_data.effective & CAP_TO_MASK(CAP_NET_RAW))  )  {
    }
    else {
        LMLOG(LCRIT, "Insufficient rights, you need CAP_NET_ADMIN and CAP_NET_RAW. See README");
        return BAD;
    }

    return GOOD;
}

#ifdef TCP_MODE
int sock_init()
{
    int sockfd;
    int backlog = 10;
    struct sockaddr_in  servaddr;

    if ( (sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0 )
    {
        perror("create socket failed.");
	    return sockfd;
    }

    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(MESHCOM_PORT);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);


    if(bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1)
    {
        perror("Bind() error.");
	    return error;
    }

    if(listen(sockfd, backlog) == -1)
    {
        LMLOG(LERR,"Listen error...");
    }

    return sockfd;
}
#else
int sock_init()
{
    printf("entry sock init!!!!");
    int sockfd;
    struct sockaddr_in  servaddr;

    if ( (sockfd = socket(AF_INET,SOCK_DGRAM,0)) < 0 )
    {
        LMLOG(LERR,"create socket failed.");
        return sockfd;
    }

    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(MESHCOM_PORT);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1)
    {
        LMLOG(LERR,"Bind() error.");
	return error;
    }

    return sockfd;
}
#endif

/*
 *  select from among readfds, the largest of which
 *  is max_fd.
 */
int have_input(int max_fd, fd_set *readfds)
{
    struct timeval tv;
    tv.tv_sec  = 0;
    tv.tv_usec = 1000000; //DEFAULT_SELECT_TIMEOUT;

    while (1)
    {
        if (select(max_fd+1,readfds,NULL,NULL,NULL) == -1) {
            if (errno == EINTR){
                continue;
            }
            else {
                LMLOG(LINF, "have_input: select error: %s", strerror(errno));
                return(BAD);
            }
        }else{
            break;
        }
    }
    return(GOOD);
}

#ifdef TCP_MODE
void event_loop()
{
    int    max_fd,clientfd;
    fd_set readfds;
    int    retval;
    socklen_t clilen;
    struct sockaddr_in cliaddr;
    int i;

    /*
     *  calculate the max_fd for select.
     */

    max_fd = msock;

	for (;;) {
        FD_ZERO(&readfds);
        FD_SET(Sock, &readfds);

        retval = have_input(max_fd, &readfds);

        if (retval != GOOD) {
            continue;        /* interrupted */
        }

        if (FD_ISSET(Sock,&readfds)){

	    clilen = sizeof(cliaddr);
            clientfd = accept(msock,(struct sockaddr *)&cliaddr,&clilen);

            max_fd = (max_fd > clientfd) ? max_fd : clientfd;
            FD_SET(clientfd, &readfds);
		}

		if (FD_ISSET(clientfd,&readfds)){
			process_ctl_msg(clientfd,AF_INET,cliaddr.sin_addr);
			FD_CLR(clientfd, &readfds);
			clientfd = 0;
			max_fd = msock;
		}
    }

}
#else
void event_loop()
{
    int    max_fd;
    fd_set readfds;
    int    retval;
    sockaddr_in sin;

    /*
     *  calculate the max_fd for select.
     */

    max_fd = msock;

    for (;;) {
        FD_ZERO(&readfds);
        FD_SET(msock, &readfds);

        retval = have_input(max_fd, &readfds);

        if (retval != GOOD) {
            continue;        /* interrupted */
        }

        if (FD_ISSET(msock,&readfds)){
            printf("Received master message");
            LMLOG(LINF,"Received master message");
            process_ctl_msg(msock,AF_INET,sin._addr);
        }
    }

}
#endif

int aud_dev_setup()
{
	return SUCCESS;
}

int main()
{
	int morehelp=0;

	while (1){
		int c;
		if ((c = getopt_long(argc, argv, "h:n:d:p:s:t:", long_option, NULL)) < 0)
		break;
		switch (c) {
		case 'h':
			morehelp++;
			break;
		case 'd':
			debug_level = atoi(optarg);
			break;
		case 'n':
			DEV_NUM = atoi(optarg);
			break;
		case 'p':
			PORT = atoi(optarg);
			break;
		case 's':
			SERVER = atoi(optarg);
		case 't':
			TRANSPORT_MODE = optarg;
		default:
			printf("Tip #0 No have the argc please input --help or read Readme\r\n");
			exit(0);
        }
    }

	if (morehelp) {
		help();

        return 0;
    }

	initial_setup();

    //handle_command_line(argc, argv);

    /* see if we need to daemonize, and if so, do it */
    demonize_start();

    /* create socket master, timer wheel, initialize interfaces */
    msock = sock_init();
    if(msock <= 0){
        LMLOG(LERR,"Error sock...");
        return error;
    }

    dev_init();

    LMLOG(LINF,"\n\n MeshCom (%s): 'meshcom-d' started... \n\n",MESHCOM_VERSION);

    /* EVENT LOOP */
    event_loop();

    /* event_loop returned: bad! */
    LMLOG(LINF, "Exiting...");
    exit_cleanup();

    return(0);
}
