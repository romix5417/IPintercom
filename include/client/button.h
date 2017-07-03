#ifndef __BUTTON_H__
#define __BUTTON_H__

#define ON_HOOK 0
#define CALL    1
#define RINGING 2

int calls_status(void);
int terminal_calls(void);
void process_button_event(void);
int button_setup(void);

#endif
