#include "display.h"
#include "gbk.h"

void Display::setup()
{
    Serial.begin(9600, SERIAL_8N1, SERIAL_TX_ONLY);
    delay(100);
}

void Display::cls()
{
    Serial.write(0x1F);
    // empty buffer
    for (unsigned short i = 0; i < LINE_SIZE; i++)
    {
        line1[i] = blankc;
        line2[i] = blankc;
    }
}

void Display::moveTo(unsigned short pos)
{
    Serial.write(0x10);
    Serial.write(pos);
}

int Display::append(String line)
{
    int writebytes = 0;
    unsigned short len = line.length();
    for (unsigned short i = 0; i < len; i++)
    {
        char p = line.charAt(i);
        Serial.print(p);
        writebytes++;
    }
    return writebytes;
}

void Display::display_waiting_connect()
{
    cls();
    display_line1(gbk_wifi_waiting_connect);
}

void Display::display_connected()
{
    cls();
    display_line1(gbk_wifi_connected);
}

void Display::display_line1(String line)
{
    moveline1();
    int right = LINE_SIZE - append(line);
    for (int i = 0; i < right; i++)
    {
        append(" ");
    }
}

void Display::display_line2(String line)
{
    moveline2();
    int right = 20 - append(line);
    for (int i = 0; i < right; i++)
    {
        append(" ");
    }
}

void Display::display_replace_line(String msg, byte *line, byte *line_buf, unsigned short offset)
{
    // empty buffer
    for (unsigned short i = 0; i < LINE_SIZE; i++)
    {
        line_buf[i] = blankc;
    }

    msg.getBytes(line_buf, LINE_SIZE + 1);

    // find first diff
    unsigned short firstDiffPos = 0;
    for (unsigned short i = 0; i < LINE_SIZE; i++)
    {
        if (line_buf[i] != line[i])
        {
            firstDiffPos = i;
            break;
        }
    }
    // copy
    for (unsigned short i = firstDiffPos; i < LINE_SIZE; i++)
    {
        line[i] = line_buf[i];
    }

    // move
    moveTo(offset + firstDiffPos);

    // display
    for (unsigned short i = firstDiffPos; i < LINE_SIZE; i++)
    {
        Serial.write(line[i]);
    }
}
