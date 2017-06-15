#ifndef __MESSAGE_H_
#define __MESSAGE_H_

typedef struct VoiceMessage{
    char filename[16];
    int  filesize;
    int  tm;
}VoiceMessage;

#endif
