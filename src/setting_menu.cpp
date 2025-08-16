// #include "setting_menu.h"
// #include "spinbox_pro.h"
// #include "version.h"


// void switch_handler(lv_event_t *e)
// {
//     lv_event_code_t code = lv_event_get_code(e);
//     // lv_obj_t * menu = lv_event_get_user_data(e);
//     lv_obj_t *menu = (lv_obj_t *)lv_event_get_user_data(e);
//     lv_obj_t *obj = lv_event_get_target(e);
//     if (code == LV_EVENT_VALUE_CHANGED)
//     {

//         if (lv_obj_has_state(obj, LV_STATE_CHECKED))
//         {
//             lv_menu_set_page(menu, NULL);
//             lv_menu_set_sidebar_page(menu, root_page);
//             lv_event_send(lv_obj_get_child(lv_obj_get_child(lv_menu_get_cur_sidebar_page(menu), 0), 0), LV_EVENT_CLICKED, NULL);
//         }
//         else
//         {
//             lv_menu_set_sidebar_page(menu, NULL);
//             lv_menu_clear_history(menu); /* Clear history because we will be showing the root page later */
//             lv_menu_set_page(menu, root_page);
//         }
//     }
// }