/* Yatao Li yatao.li@live.com 2019 */

#ifndef SDDRIVEPAGE_H__
#define SDDRIVEPAGE_H__

#include "FileBrowserPage.h"

class SDDrivePage : public FileBrowserPage {
public:

  SDDrivePage(Encoder *e1 = NULL, Encoder *e2 = NULL, Encoder *e3 = NULL,
            Encoder *e4 = NULL)
      : FileBrowserPage(e1, e2, e3, e4) { }

  bool handleEvent(gui_event_t *event);
  void init();
  void setup();
  void save_snapshot();
  void load_snapshot();
};

extern SDDrivePage sddrive_page;

#endif /* SDDRIVEPAGE_H__ */
