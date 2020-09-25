/* Justin Mammarella jmamma@gmail.com 2018 */

#ifndef GRIDWRITEPAGE_H__
#define GRIDWRITEPAGE_H__

#include "GridIOPage.h"

#define WRITE_PAGE 0
#define CHAIN_PAGE 1

class GridWritePage : public GridIOPage {
 public:
 uint8_t mode = WRITE_PAGE;
 GridWritePage(Encoder *e1 = NULL, Encoder *e2 = NULL, Encoder *e3 = NULL, Encoder *e4 = NULL) : GridIOPage(e1, e2, e3 ,e4) {

 }
 void send_chain();
 bool handleEvent(gui_event_t *event);
 void draw_popup();
 void display();
 void setup();
 void init();
 void cleanup();
};

#endif /* GRIDWRITEPAGE_H__ */
