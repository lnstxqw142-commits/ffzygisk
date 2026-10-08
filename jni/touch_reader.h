#ifndef TOUCH_READER_H
#define TOUCH_READER_H

void touchReaderStart();
void touchReaderStop();

typedef void (*TouchCallback)(int action, float x, float y);
void touchReaderSetCallback(TouchCallback cb);

#endif
