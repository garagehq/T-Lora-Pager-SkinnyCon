/**
 * @file      ui_badgeshark.cpp
 * @brief     BadgeShark — LoRa packet sniffer inspired by Supercon 2025 badge
 * @details   Wireshark-style LoRa packet monitor. Shows incoming packets with
 *            RSSI, SNR, length, and hex dump. Auto-scrolling table with stats.
 *            Ported from Supercon 2025 badge BadgeShark app concept.
 */
#include "ui_define.h"
#include "ui_skinnycon_theme.h"

#define SHARK_BG          SC_BG_DARK     /* Terminal-style dark bg */
#define SHARK_GREEN       SC_GREEN_BRIGHT
#define SHARK_YELLOW      SC_ACCENT
#define SHARK_CYAN        SC_CYAN
#define SHARK_WHITE       SC_TEXT
#define SHARK_DARK_PANEL  SC_PANEL_ALT
#define MAX_PACKETS       50

LV_FONT_DECLARE(font_alibaba_12);
LV_FONT_DECLARE(font_alibaba_24);

static lv_obj_t *shark_cont = NULL;
static lv_obj_t *stats_label = NULL;
static lv_obj_t *packet_list = NULL;
static lv_timer_t *rx_timer = NULL;

static uint32_t total_packets = 0;
static uint32_t total_bytes = 0;
static float last_rssi = 0;
static float last_snr = 0;

static void shark_add_packet(const char *hex_preview, int len, float rssi, float snr)
{
    total_packets++;
    total_bytes += len;
    last_rssi = rssi;
    last_snr = snr;

    /* Add row to packet list */
    lv_obj_t *row = lv_obj_create(packet_list);
    lv_obj_set_size(row, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_color(row, (total_packets % 2) ? SHARK_DARK_PANEL : SHARK_BG, 0);
    lv_obj_set_style_bg_opa(row, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(row, 0, 0);
    lv_obj_set_style_radius(row, 0, 0);
    lv_obj_set_style_pad_ver(row, 2, 0);
    lv_obj_set_style_pad_hor(row, 6, 0);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);

    /* Packet number */
    lv_obj_t *num = lv_label_create(row);
    lv_label_set_text_fmt(num, "#%lu", (unsigned long)total_packets);
    lv_obj_set_style_text_color(num, SHARK_CYAN, 0);
    lv_obj_set_style_min_width(num, 50, 0);

    /* Length */
    lv_obj_t *len_lbl = lv_label_create(row);
    lv_label_set_text_fmt(len_lbl, "%dB", len);
    lv_obj_set_style_text_color(len_lbl, SHARK_YELLOW, 0);
    lv_obj_set_style_min_width(len_lbl, 40, 0);

    /* RSSI */
    lv_obj_t *rssi_lbl = lv_label_create(row);
    lv_label_set_text_fmt(rssi_lbl, "%.0fdBm", rssi);
    lv_obj_set_style_text_color(rssi_lbl, (rssi > -80) ? SHARK_GREEN : SC_RED, 0);
    lv_obj_set_style_min_width(rssi_lbl, 65, 0);

    /* Hex preview */
    lv_obj_t *hex_lbl = lv_label_create(row);
    lv_label_set_text(hex_lbl, hex_preview);
    lv_obj_set_style_text_color(hex_lbl, SHARK_WHITE, 0);
    lv_label_set_long_mode(hex_lbl, LV_LABEL_LONG_CLIP);
    lv_obj_set_flex_grow(hex_lbl, 1);

    /* Auto-scroll to bottom */
    lv_obj_scroll_to_y(packet_list, LV_COORD_MAX, LV_ANIM_ON);

    /* Limit visible packets */
    while (lv_obj_get_child_count(packet_list) > MAX_PACKETS) {
        lv_obj_del(lv_obj_get_child(packet_list, 0));
    }

    /* Update stats */
    if (stats_label) {
        lv_label_set_text_fmt(stats_label,
            "Packets: %lu  |  Bytes: %lu  |  RSSI: %.0f dBm  |  SNR: %.1f dB",
            (unsigned long)total_packets, (unsigned long)total_bytes, last_rssi, last_snr);
    }
}

static void shark_rx_task(lv_timer_t *t)
{
#ifdef ARDUINO
    radio_rx_params_t rx_params;
    uint8_t recv_buf[256];
    rx_params.data = recv_buf;
    rx_params.length = sizeof(recv_buf);

    hw_get_radio_rx(rx_params);
    if (rx_params.state == 0 && rx_params.length > 0) {
        /* Build hex preview (first 16 bytes) */
        char hex[50];
        int preview_len = rx_params.length > 16 ? 16 : rx_params.length;
        int pos = 0;
        for (int i = 0; i < preview_len && pos < 48; i++) {
            pos += snprintf(hex + pos, sizeof(hex) - pos, "%02X ", recv_buf[i]);
        }
        if (rx_params.length > 16) strcat(hex, "...");

        shark_add_packet(hex, rx_params.length, rx_params.rssi, rx_params.snr);
        hw_feedback();
        hw_set_radio_listening();
    }
#endif
}

static void shark_exit(lv_obj_t *parent);  /* forward decl for ESC handler */

static void shark_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED || code == LV_EVENT_LONG_PRESSED) {
        shark_exit(NULL);
        menu_show();
        return;
    }
    if (code != LV_EVENT_KEY) return;
    if (lv_event_get_key(e) == LV_KEY_ESC) {
        shark_exit(NULL);
        menu_show();
    }
}

static void shark_setup(lv_obj_t *parent)
{
    total_packets = 0;
    total_bytes = 0;

    shark_cont = lv_obj_create(parent);
    lv_obj_set_size(shark_cont, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(shark_cont, SHARK_BG, 0);
    lv_obj_set_style_bg_opa(shark_cont, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(shark_cont, 0, 0);
    lv_obj_set_style_radius(shark_cont, 0, 0);
    lv_obj_set_style_pad_all(shark_cont, 0, 0);
    lv_obj_set_flex_flow(shark_cont, LV_FLEX_FLOW_COLUMN);

    /* Header bar — Supercon-style infobar */
    lv_obj_t *header = lv_obj_create(shark_cont);
    lv_obj_set_size(header, LV_PCT(100), 28);
    lv_obj_set_style_bg_color(header, SC_HEADER, 0);
    lv_obj_set_style_bg_opa(header, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(header, 0, 0);
    lv_obj_set_style_radius(header, 0, 0);
    lv_obj_set_style_pad_hor(header, 8, 0);
    lv_obj_set_style_pad_ver(header, 4, 0);

    lv_obj_t *title = lv_label_create(header);
    lv_label_set_text(title, LV_SYMBOL_EYE_OPEN " BadgeShark");
    lv_obj_set_style_text_color(title, SC_ACCENT, 0);
    lv_obj_set_style_text_font(title, &font_alibaba_12, 0);
    lv_obj_align(title, LV_ALIGN_LEFT_MID, 0, 0);

    /* Stats bar */
    stats_label = lv_label_create(header);
    lv_label_set_text(stats_label, "Listening...");
    lv_obj_set_style_text_color(stats_label, SHARK_WHITE, 0);
    lv_obj_align(stats_label, LV_ALIGN_RIGHT_MID, 0, 0);

    /* Column headers */
    lv_obj_t *col_header = lv_obj_create(shark_cont);
    lv_obj_set_size(col_header, LV_PCT(100), 20);
    lv_obj_set_style_bg_color(col_header, SC_BORDER, 0);
    lv_obj_set_style_bg_opa(col_header, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(col_header, 0, 0);
    lv_obj_set_style_radius(col_header, 0, 0);
    lv_obj_set_style_pad_hor(col_header, 6, 0);
    lv_obj_set_style_pad_ver(col_header, 2, 0);
    lv_obj_set_flex_flow(col_header, LV_FLEX_FLOW_ROW);

    const char *cols[] = {"#", "Len", "RSSI", "Data"};
    int col_widths[] = {50, 40, 65, 0};
    for (int i = 0; i < 4; i++) {
        lv_obj_t *c = lv_label_create(col_header);
        lv_label_set_text(c, cols[i]);
        lv_obj_set_style_text_color(c, SC_TEXT, 0);
        if (col_widths[i] > 0) {
            lv_obj_set_style_min_width(c, col_widths[i], 0);
        } else {
            lv_obj_set_flex_grow(c, 1);
        }
    }

    /* Packet list (scrollable) */
    packet_list = lv_obj_create(shark_cont);
    lv_obj_set_size(packet_list, LV_PCT(100), LV_PCT(100));
    lv_obj_set_flex_grow(packet_list, 1);
    lv_obj_set_style_bg_color(packet_list, SHARK_BG, 0);
    lv_obj_set_style_bg_opa(packet_list, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(packet_list, 0, 0);
    lv_obj_set_style_radius(packet_list, 0, 0);
    lv_obj_set_style_pad_all(packet_list, 0, 0);
    lv_obj_set_flex_flow(packet_list, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_scrollbar_mode(packet_list, LV_SCROLLBAR_MODE_AUTO);
    lv_obj_set_scroll_dir(packet_list, LV_DIR_VER);

    /* Register input for back button */
    lv_obj_add_event_cb(shark_cont, shark_event_cb, LV_EVENT_KEY, NULL);
    lv_obj_add_event_cb(shark_cont, shark_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(shark_cont, shark_event_cb, LV_EVENT_LONG_PRESSED, NULL);
    lv_group_t *g = lv_group_get_default();
    if (g) lv_group_add_obj(g, shark_cont);

    /* Start listening */
#ifdef ARDUINO
    hw_set_radio_listening();
    rx_timer = lv_timer_create(shark_rx_task, 200, NULL);
#else
    /* Demo data for simulation */
    shark_add_packet("07 E9 3A 1F FF FF FF FF 00 01 02 03 06 01 48 65", 32, -45.0, 8.5);
    shark_add_packet("07 E9 7B 2E FF FF FF FF 00 04 05 06 06 02 4F 4B", 28, -72.0, 4.2);
    shark_add_packet("07 E9 A1 00 00 01 02 03 00 04 05 06 01 03 50 49", 16, -95.0, -1.0);
    shark_add_packet("07 E9 12 44 FF FF FF FF 00 0A 0B 0C 07 01 53 69", 48, -58.0, 6.8);
    shark_add_packet("07 E9 55 67 FF FF FF FF 00 07 08 09 06 03 48 69", 24, -83.0, 3.1);
#endif
}

static void shark_exit(lv_obj_t *parent)
{
    if (rx_timer) {
        lv_timer_del(rx_timer);
        rx_timer = NULL;
    }
    if (shark_cont) {
        lv_obj_del(shark_cont);
        shark_cont = NULL;
    }
    stats_label = NULL;
    packet_list = NULL;
}

app_t ui_badgeshark_main = {shark_setup, shark_exit, NULL};
