#ifndef UI_NAMETAG_H
#define UI_NAMETAG_H

#ifdef __cplusplus
extern "C" {
#endif

void ui_nametag_init(void);
void ui_nametag_set_text(const char *text);
void ui_nametag_update(void);

#ifdef __cplusplus
}
#endif

#endif /* UI_NAMETAG_H */
