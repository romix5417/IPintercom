#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// 配置结构体
struct configItem
{
	char key[20];
	char value[50];
}  configList[] =	{
	{"host_ip", 0},
    {"host_num", 0},
	{"dev_num", 0},
	{"debug_level", 0},
	{"dest_port", 0},
	{"local_port", 0},
    {"ftp_port", 0}
};

/*
 * 字符串中寻找以等号分开的键值对
 * @param  src	源字符串 [输入参数]
 * @param  key	   键    [输出参数]
 * @param value	   值    [输出参数]
 */
int strkv(char *src, char *key, char *value)
{
	char *p,*q;
	int len;
	p = strchr(src, '=');	// p找到等号
	q = strchr(src, '\0');	// q找到换行

	// 如果有等号有换行
	if (p != NULL && q != NULL)
	{
		*q = '\0'; // 将换行设置为字符串结尾
		strncpy(key, src, p - src); // 将等号前的内容拷贝到 key 中
		strcpy(value, p+1);	// 将等号后的内容拷贝到 value 中
		return 1;
	}else
	{
		return 0;
	}
}

/*
 * 配置函数
 * @param configFilePath 配置文件路径 [输入参数]
 * @param   configVar	 存储配置的值 [输出参数]
 * @param   configNum	 需配置的个数 [输入参数]
 */
void config(char *configFilePath, struct configItem * configVar, int configNum)
{
	int i;
	FILE *fd;
	char buf[50]="";	// 缓冲字符串
	char key[50]="";	// 配置变量名
	char value[50]="";	// 配置变量值	

	// 打开配置文件
	fd = fopen(configFilePath, "r");

	if (fd == NULL)
	{
		printf("配置文件打开失败！\n");
		system("pause");
		exit(-1);
	}

	// 依次读取文件的每一行
	while(fgets(buf, 50, fd))
	{
		// 读取键值对
		if (strkv(buf, key, value))
		{
			// 读取成功则循环与配置数组比较
			for(i = 0; i< configNum; i++)
			{
				// 名称相等则拷贝
				if (strcmp(key, configVar[i].key) == 0)
				{
					strcpy(configVar[i].value, value);
				}
			}
			// 清空 读取出来的 key
			memset(key, 0, strlen(key));
		}
	}

	fclose(fd);
}

int main()
{
	int i;
	
	config(
		"IPinterCom.cfg",	// 配置文件路径
		configList,			// 配置变量数组
		sizeof(configList)/sizeof(struct configItem) // 数组大小
	);

	// 循环打印获取到的值
	for(i=0; i<sizeof(configList)/sizeof(struct configItem); i++)
	{
		printf("%s = %s \n", configList[i].key, configList[i].value);
	}

	return 0;
}
