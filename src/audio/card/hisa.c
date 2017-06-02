#include<audio/aud.h>

static int hisa_card_detect(AudSndCard *card)
{
    return;
}

static int hisa_card_init(AudSndCard *card)
{
    return;
}

static void hisa_card_create_reader(AudSndCard *card)
{

}

static void hisa_card_create_writer(AudSndCard *card)
{
    return;
}

static void hisa_card_unload(AudSndCard *card)
{
    return;
}

SndCardDesc hisa_card_desc = {
    .detect = hisa_card_detect,
    .init   = hisa_card_init,
    .create_reader = hisa_card_create_reader,
    .create_writer = hisa_card_create_writer,
    .unload = hisa_card_unload
};
