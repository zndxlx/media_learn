

#include <ortp/ortp.h>
#include <signal.h>
#include <stdlib.h>
#ifndef _WIN32
#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#endif

int runcond = 1;

void stophandler(int signum)
{
    runcond = 0;
}

static const char *help = "usage: rtpsend	filename dest_ip4addr dest_port [ --with-clockslide <value> ] [ --with-jitter <milliseconds>]\n";

const int AU_HEADER_SIZE = 4;

int main(int argc, char *argv[])
{
    RtpSession *session;
    unsigned char buffer[4096];
    int i;
    FILE *infile;
    char *ssrc;
    uint32_t user_ts = 0;
    int clockslide = 0;
    // int jitter = 0;
    if (argc < 4)
    {
        printf("%s", help);
        return -1;
    }
    for (i = 4; i < argc; i++)
    {
        if (strcmp(argv[i], "--with-clockslide") == 0)
        {
            i++;
            if (i >= argc)
            {
                printf("%s", help);
                return -1;
            }
            clockslide = atoi(argv[i]);
            ortp_message("Using clockslide of %i milisecond every 50 packets.", clockslide);
        }
        else if (strcmp(argv[i], "--with-jitter") == 0)
        {
            ortp_message("Jitter will be added to outgoing stream.");
            i++;
            if (i >= argc)
            {
                printf("%s", help);
                return -1;
            }
            //jitter = atoi(argv[i]);
        }
    }

    ortp_init();
    ortp_scheduler_init();
    ortp_set_log_level_mask(NULL, ORTP_MESSAGE | ORTP_WARNING | ORTP_ERROR);
    session = rtp_session_new(RTP_SESSION_SENDONLY);

    rtp_session_set_scheduling_mode(session, 1);
    rtp_session_set_blocking_mode(session, 1);
    //rtp_session_set_connected_mode(session,TRUE);
    rtp_session_set_connected_mode(session, FALSE); //需要调整为FALSE,不然会死机
    rtp_session_set_remote_addr(session, argv[2], atoi(argv[3]));
    //rtp_session_set_payload_type(session,0);

    PayloadType payload_type_aac = {
        PAYLOAD_AUDIO_PACKETIZED, /*type */
        48000,                    /*clock rate */
        0,                        /* bytes per sample N/A */
        NULL,                     /* zero pattern N/A*/
        0,                        /*pattern_length N/A */
        64000,                    /*	normal_bitrate */
        "mpeg4-generic",          /* MIME subtype */
        2,                        /* Audio Channels */
        0                         /*flags */
    };

    rtp_profile_set_payload(&av_profile, 97, &payload_type_aaceld_48k);
    rtp_session_set_payload_type(session, 97); //测试的是pcma payloadtype 设置为8
    ssrc = getenv("SSRC");
    if (ssrc != NULL)
    {
        printf("using SSRC=%i.\n", atoi(ssrc));
        rtp_session_set_ssrc(session, atoi(ssrc));
    }

#ifndef _WIN32
    infile = fopen(argv[1], "r");
#else
    infile = fopen(argv[1], "rb");
#endif

    if (infile == NULL)
    {
        perror("Cannot open file");
        return -1;
    }

    signal(SIGINT, stophandler);

    //读取7个字节的adts头
    while (((i = fread(buffer, 1, 7, infile)) > 0) && (runcond))
    {
        printf("i=%d\n", i);
        if (i != 7)
        {
            printf("file end\n");
            break;
        }

        if (!((buffer[0] == 0xFF) && ((buffer[1] & 0xF6) == 0xF0)))
        {
            printf("format error\n");
            break;
        }
        int frame_length = ((((unsigned int)buffer[3] & 0x3)) << 11) | (((unsigned int)buffer[4]) << 3) | (buffer[5] >> 5);
        printf("frame_length=%d\n", frame_length);
        int bodylen = fread(buffer + 4, 1, frame_length - 7, infile);
        if (bodylen != frame_length - 7)
        {
            printf("body not enough bodylen=%d, frame_length=%d\n", bodylen, frame_length);
            break;
        }

        buffer[0] = 0x00;
        buffer[1] = 0x10;
        buffer[2] = ((frame_length - 7) & 0x1fe0) >> 5;
        buffer[3] = ((frame_length - 7) & 0x1f) << 3;

        rtp_session_send_with_ts(session, buffer, frame_length - 7 + 4, user_ts);
        user_ts += 1024;
    }

    fclose(infile);
    rtp_session_destroy(session);
    ortp_exit();
    ortp_global_stats_display();

    return 0;
}
