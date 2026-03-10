/**
 * @file      ui_nettools.cpp
 * @brief     Net Tools — LoRa mesh diagnostics inspired by Supercon 2025 badge
 * @details   Ping/pong latency tester and mesh node discovery over LoRa.
 *            Shows round-trip time, packet loss stats, and discovered peers.
 *            Ported from Supercon 2025 badge network diagnostic concept.
 */
#include "ui_define.h"

#define NET_BG            lv_color_hex(0x0D1117)
#define NET_GREEN         lv_color_hex(0x39D353)
#define NET_YELLOW        lv_color_hex(0xE39810)
#define NET_CYAN          lv_color_hex(0x58A6FF)
#define NET_WHITE         lv_color_hex(0xE6EDF3)
#define NET_RED           lv_color_hex(0xFF6B6B)
#define NET_PANEL         lv_color_hex(0x161B22)
#define MAX_PING_LOG      20

LV_FONT_DECLARE(font_alibaba_12);
LV_FONT_DECLARE(font_alibaba_24);

static lv_obj_t *net_cont = NULL;
static lv_obj_t *ping_log = NULL;
static lv_obj_t *net_stats_label = NULL;
static lv_timer_t *ping_timer = NULL;

static uint32_t pings_sent = 0;
static uint32_t pings_recv = 0;
static uint32_t total_rtt = 0;
static uint32_t min_rtt = 9999;
static uint32_t max_rtt = 0;

static void net_add_ping_result(uint32_t rtt_ms, bool success)
{
    pings_sent++;
    if (success) {
        pings_recv++;
        total_rtt += rtt_ms;
        if (rtt_ms < min_rtt) min_rtt = rtt_ms;
        if (rtt_ms > max_rtt) max_rtt = rtt_ms;
    }

    /* Add log entry */
    lv_obj_t *entry = lv_label_create(ping_log);
    if (success) {
        lv_label_set_text_fmt(entry, "PONG #%lu  RTT: %lu ms", (unsigned long)pings_sent, (unsigned long)rtt_ms);
        lv_obj_set_style_text_color(entry, (rtt_ms < 200) ? NET_GREEN : NET_YELLOW, 0);
    } else {
        lv_label_set_text_fmt(entry, "PING #%lu  TIMEOUT", (unsigned long)pings_sent);
        lv_obj_set_style_text_color(entry, NET_RED, 0);
    }

    lv_obj_scroll_to_y(ping_log, LV_COORD_MAX, LV_ANIM_ON);

    /* Limit entries */
    while (lv_obj_get_child_count(ping_log) > MAX_PING_LOG) {
        lv_obj_del(lv_obj_get_child(ping_log, 0));
    }

    /* Update stats */
    if (net_stats_label) {
        uint32_t loss_pct = pings_sent > 0 ? ((pings_sent - pings_recv) * 100 / pings_sent) : 0;
        uint32_t avg_rtt = pings_recv > 0 ? (total_rtt / pings_recv) : 0;
        lv_label_set_text_fmt(net_stats_label,
            "Sent: %lu  |  Recv: %lu  |  Loss: %lu%%  |  Avg: %lu ms",
            (unsigned long)pings_sent, (unsigned long)pings_recv,
            (unsigned long)loss_pct, (unsigned long)avg_rtt);
    }
}

static void net_ping_task(lv_timer_t *t)
{
#ifdef ARDUINO
    /* Real LoRa ping/pong would go here */
    /* Send ping packet, measure RTT on response */
    static uint32_t seq = 0;
    seq++;
    /* Placeholder — real implementation would use hw_get_radio_rx */
    net_add_ping_result(0, false);
#endif
}

static void net_setup(lv_obj_t *parent)
{
    pings_sent = 0;
    pings_recv = 0;
    total_rtt = 0;
    min_rtt = 9999;
    max_rtt = 0;

    net_cont = lv_obj_create(parent);
    lv_obj_set_size(net_cont, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(net_cont, NET_BG, 0);
    lv_obj_set_style_bg_opa(net_cont, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(net_cont, 0, 0);
    lv_obj_set_style_radius(net_cont, 0, 0);
    lv_obj_set_style_pad_all(net_cont, 0, 0);
    lv_obj_set_flex_flow(net_cont, LV_FLEX_FLOW_COLUMN);

    /* Header */
    lv_obj_t *header = lv_obj_create(net_cont);
    lv_obj_set_size(header, LV_PCT(100), 28);
    lv_obj_set_style_bg_color(header, lv_color_hex(0x21262D), 0);
    lv_obj_set_style_bg_opa(header, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(header, 0, 0);
    lv_obj_set_style_radius(header, 0, 0);
    lv_obj_set_style_pad_hor(header, 8, 0);

    lv_obj_t *title = lv_label_create(header);
    lv_label_set_text(title, LV_SYMBOL_WIFI " Net Tools");
    lv_obj_set_style_text_color(title, NET_GREEN, 0);
    lv_obj_set_style_text_font(title, &font_alibaba_12, 0);
    lv_obj_align(title, LV_ALIGN_LEFT_MID, 0, 0);

    /* Stats bar */
    net_stats_label = lv_label_create(header);
    lv_label_set_text(net_stats_label, "Ready — press to ping");
    lv_obj_set_style_text_color(net_stats_label, NET_WHITE, 0);
    lv_obj_align(net_stats_label, LV_ALIGN_RIGHT_MID, 0, 0);

    /* Two-column layout: ping log (left) + peer list (right) */
    lv_obj_t *body = lv_obj_create(net_cont);
    lv_obj_set_size(body, LV_PCT(100), LV_PCT(100));
    lv_obj_set_flex_grow(body, 1);
    lv_obj_set_style_bg_opa(body, LV_OPA_0, 0);
    lv_obj_set_style_border_width(body, 0, 0);
    lv_obj_set_style_radius(body, 0, 0);
    lv_obj_set_style_pad_all(body, 4, 0);
    lv_obj_set_flex_flow(body, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_column(body, 4, 0);

    /* Ping log panel */
    lv_obj_t *ping_panel = lv_obj_create(body);
    lv_obj_set_size(ping_panel, LV_PCT(60), LV_PCT(100));
    lv_obj_set_flex_grow(ping_panel, 3);
    lv_obj_set_style_bg_color(ping_panel, NET_PANEL, 0);
    lv_obj_set_style_bg_opa(ping_panel, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(ping_panel, lv_color_hex(0x30363D), 0);
    lv_obj_set_style_border_width(ping_panel, 1, 0);
    lv_obj_set_style_radius(ping_panel, 4, 0);
    lv_obj_set_style_pad_all(ping_panel, 4, 0);
    lv_obj_set_flex_flow(ping_panel, LV_FLEX_FLOW_COLUMN);

    lv_obj_t *ping_title = lv_label_create(ping_panel);
    lv_label_set_text(ping_title, "PING LOG");
    lv_obj_set_style_text_color(ping_title, NET_CYAN, 0);

    ping_log = lv_obj_create(ping_panel);
    lv_obj_set_size(ping_log, LV_PCT(100), LV_PCT(100));
    lv_obj_set_flex_grow(ping_log, 1);
    lv_obj_set_style_bg_opa(ping_log, LV_OPA_0, 0);
    lv_obj_set_style_border_width(ping_log, 0, 0);
    lv_obj_set_style_pad_all(ping_log, 0, 0);
    lv_obj_set_flex_flow(ping_log, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_scrollbar_mode(ping_log, LV_SCROLLBAR_MODE_AUTO);

    /* Peer discovery panel */
    lv_obj_t *peer_panel = lv_obj_create(body);
    lv_obj_set_size(peer_panel, LV_PCT(40), LV_PCT(100));
    lv_obj_set_flex_grow(peer_panel, 2);
    lv_obj_set_style_bg_color(peer_panel, NET_PANEL, 0);
    lv_obj_set_style_bg_opa(peer_panel, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(peer_panel, lv_color_hex(0x30363D), 0);
    lv_obj_set_style_border_width(peer_panel, 1, 0);
    lv_obj_set_style_radius(peer_panel, 4, 0);
    lv_obj_set_style_pad_all(peer_panel, 4, 0);
    lv_obj_set_flex_flow(peer_panel, LV_FLEX_FLOW_COLUMN);

    lv_obj_t *peer_title = lv_label_create(peer_panel);
    lv_label_set_text(peer_title, "PEERS");
    lv_obj_set_style_text_color(peer_title, NET_CYAN, 0);

#ifndef ARDUINO
    /* Demo data for simulation */
    net_add_ping_result(45, true);
    net_add_ping_result(62, true);
    net_add_ping_result(0, false);
    net_add_ping_result(38, true);
    net_add_ping_result(125, true);
    net_add_ping_result(52, true);
    net_add_ping_result(0, false);
    net_add_ping_result(41, true);

    /* Demo peers */
    const char *peers[] = {"Badge-0x3A1F", "Badge-0x7B2E", "Badge-0xA100", "Badge-0x1244"};
    const int rssis[] = {-45, -72, -95, -58};
    for (int i = 0; i < 4; i++) {
        lv_obj_t *peer = lv_label_create(peer_panel);
        lv_label_set_text_fmt(peer, "%s\n  %d dBm", peers[i], rssis[i]);
        lv_obj_set_style_text_color(peer, (rssis[i] > -80) ? NET_GREEN : NET_YELLOW, 0);
    }
#else
    hw_set_radio_listening();
    ping_timer = lv_timer_create(net_ping_task, 2000, NULL);
#endif
}

static void net_exit(lv_obj_t *parent)
{
    if (ping_timer) {
        lv_timer_del(ping_timer);
        ping_timer = NULL;
    }
    if (net_cont) {
        lv_obj_del(net_cont);
        net_cont = NULL;
    }
    net_stats_label = NULL;
    ping_log = NULL;
}

app_t ui_nettools_main = {net_setup, net_exit, NULL};
