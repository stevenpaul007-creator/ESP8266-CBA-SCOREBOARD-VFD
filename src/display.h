#pragma once
#include <Arduino.h>

const static unsigned short LINE_SIZE = 20;
const static char blankc = ' ';

inline byte line1[LINE_SIZE] = {32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32};
inline byte line2[LINE_SIZE] = {32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32};

inline byte line1_buf[LINE_SIZE] = {32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32};
inline byte line2_buf[LINE_SIZE] = {32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32};

class Display
{
public:
    static void setup();
    static void cls();
    static void moveTo(unsigned short pos);
    static void moveline1() { moveTo(0); }
    static void moveline2() { moveTo(LINE_SIZE); }
    static int append(String line);
    static void display_line1(String line);
    static void display_line2(String line);
    static void display_replace_line(String msg, byte *line, byte *line_buf, unsigned short offset);
    static void display_replace_line1(String line)
    {
        display_replace_line(line, line1, line1_buf, 0);
    }

    static void display_replace_line2(String line)
    {
        display_replace_line(line, line2, line2_buf, 20);
    }

    static void display_replace_line(int index, String line)
    {
        if (index)
        {
            display_replace_line2(line);
        }
        else
        {
            display_replace_line1(line);
        }
    }
    static void display_waiting_connect();
    static void display_connected();
};