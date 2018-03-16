#include "ProjectPages.h"

extern MCLEncoder loadproj_param1(1, 64, ENCODER_RES_SYS);

extern MCLEncoder newproj_param1(1, 10, ENCODER_RES_SYS);
extern MCLEncoder newproj_param2(0, 36, ENCODER_RES_SYS);
extern MCLEncoder newproj_param4(0, 127, ENCODER_RES_SYS);


extern NewProjectPage new_project_page(&newproj_param1, &newproj_param2);

extern LoadProjectPage load_project_page(&loadproj_param1);
