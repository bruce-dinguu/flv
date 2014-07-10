#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

int main(int argc, char **argv)
{
    FILE *fd, *fp;
    struct stat info;
    char * buff;
    int i, length;
    int tmp, tagsize = -11;
    int cnt[4] = {0, 0, 0, 0};

    fd = fopen(argv[1], "rb");
    if (fd == NULL)
    {
        printf("open file fail\n");
        return -1;
    }
    fp = fopen(argv[2], "wb");
    if (fp == NULL)
    {
        printf("open file fail\n");
        fclose(fp);
        return -1;
    }
    printf("open file OK\n");

    fstat(fileno(fd), &info);
    length = info.st_size;
    fprintf(fp, "file length[0x%x][%d]\n", length, length);

    buff = malloc(length);
    if (buff != NULL)
    {
        fprintf(fp, "malloc OK\n");

        fread(buff, length, 1, fd);
        fprintf(fp, "[%X][%X][%X]\n", buff[0], buff[1], buff[2]);    //FLV
        if ((buff[0] == 0x46) && (buff[1] == 0x4C) && (buff[2] == 0x56))
        {
            i = buff[8];
            fprintf(fp, "head length[%d]\n", i);
            fprintf(fp, "[cur-posi] [pre-size] [cur-type] [cur-size] [cur-time]\n");
            for (; i < length; i)
            {

                fprintf(fp, "[%08X] ", i);
                tmp = ((int)(buff[i]&0xFF)<<24)+((int)(buff[i+1]&0xFF)<<16)+((int)(buff[i+2]&0xFF)<<8)+(buff[i+3]&0xFF);
                fprintf(fp, "[%08X] ", tmp);
                if (tmp != (tagsize+11))
                {
                    fprintf(fp, "ERR(0x%x, 0x%x)\n", tmp, tagsize);
                    break;
                }
                i+=4;

                if (i >= length)
                {
                    fprintf(fp, "EOF\n");
                    break;
                }

                if (buff[i] == 0x12)
                    cnt[0]++;
                else if (buff[i] == 0x9)
                    cnt[1]++;
                else if (buff[i] == 0x8)
                    cnt[2]++;
                else
                    cnt[3]++;

                fprintf(fp, "[%08X] ", buff[i]);
                i+=1;

                tagsize = ((int)(buff[i]&0xFF)<<16)+((int)(buff[i+1]&0xFF)<<8)+(buff[i+2]&0xFF);
                fprintf(fp, "[%08X] ", tagsize);
                i+=3;

                tmp = ((int)(buff[i+3]&0xFF)<<24)+((int)(buff[i]&0xFF)<<16)+((int)(buff[i+1]&0xFF)<<8)+(buff[i+2]&0xFF);
                fprintf(fp, "[%08X][%08d] ", tmp, tmp);
                i+=4;

                i+=3;

                fprintf(fp, "[%02X]\n",buff[i]&0xFF);
                i+=tagsize;
            }
        }
        else
        {
            fprintf(fp, "NOT FLV!\n");
        }

        free(buff);
    }
    else
    {
        fprintf(fp, "malloc fail\n");
    }

    fprintf(fp, "[%08d] [%08d] [%08d] [%08d]", cnt[0], cnt[1], cnt[2], cnt[3]);
    fclose(fd);
    fclose(fp);
    printf("convert file OK\n");

    return 0;
}
