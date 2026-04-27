#!/usr/bin/env python3
"""generate source/m57/keymaps/via/borne_status.h from borne_status.toml.

reads the lighting rule spec next to the keymap and lowers it into the
QMK rgb_matrix custom-effect implementation included by rgb_matrix_user.inc.
positions in the toml are absolute LED indices (0..57), not key labels, so
the generator output is independent of the live vial keymap.

resolution model encoded into the generated code:
- priority 0 rules are emitted before the per-layer switch and apply on
  every frame regardless of active layer
- priority > 0 rules are emitted inside the matching `case <layer>:` and
  inside the `default:` branch for any rule with `layers = "all"`
- within a case, rules are emitted in ascending priority so higher-priority
  rules overwrite lower ones via successive rgb_matrix_set_color calls

usage:  scripts/borne_status_gen.py
"""

from __future__ import annotations

import sys
import tomllib
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
TOML_PATH = ROOT / "source/m57/keymaps/via/borne_status.toml"
OUT_PATH = ROOT / "source/m57/keymaps/via/borne_status.h"

GATE_C_EXPR = {
    "num_lock_on":    "leds.num_lock",
    "caps_lock_on":   "leds.caps_lock",
    "scroll_lock_on": "leds.scroll_lock",
}


def color_components(rgb, pulsating):
    """lower a static RGB triple to three C expressions, scaled by `br` if pulsating."""
    out = []
    for v in rgb:
        if not pulsating:
            out.append(str(v))
        elif v == 0:
            out.append("0")
        elif v == 255:
            out.append("br")
        else:
            out.append(f"scale8({v}, br)")
    return tuple(out)


def emit_rule(rule, groups, colors, lines, indent):
    leds_field = rule["leds"]
    leds_list = groups[leds_field] if isinstance(leds_field, str) else leds_field
    gate = rule.get("gate")
    sub = indent
    if gate is not None:
        if gate not in GATE_C_EXPR:
            raise SystemExit(f"unknown gate {gate!r} on rule {rule.get('id')!r}")
        lines.append(f"{indent}if ({GATE_C_EXPR[gate]}) {{")
        sub = indent + "    "

    color_field = rule["color"]
    if color_field == "@mod_blend":
        pulsate = "true" if rule.get("pulsating") else "false"
        lines.append(f"{sub}{{")
        lines.append(f"{sub}    uint8_t mr, mg, mb;")
        lines.append(f"{sub}    borne_compute_mod_blend(br, {pulsate}, &mr, &mg, &mb);")
        for led in leds_list:
            lines.append(f"{sub}    borne_set({led}, led_min, led_max, mr, mg, mb);")
        lines.append(f"{sub}}}")
    else:
        if color_field not in colors:
            raise SystemExit(f"unknown color {color_field!r} on rule {rule.get('id')!r}")
        r, g, b = color_components(colors[color_field], rule.get("pulsating", False))
        # all_leds bg gets a loop; other rules emit per-LED calls.
        if leds_field == "all_leds" and not gate:
            lines.append(f"{sub}for (uint8_t i = led_min; i < led_max; i++) rgb_matrix_set_color(i, {r}, {g}, {b});")
        else:
            for led in leds_list:
                lines.append(f"{sub}borne_set({led}, led_min, led_max, {r}, {g}, {b});")

    if gate is not None:
        lines.append(f"{indent}}}")


def main():
    with TOML_PATH.open("rb") as f:
        cfg = tomllib.load(f)
    groups = cfg.get("groups", {})
    colors = cfg.get("colors", {})
    rules = cfg.get("rule", [])

    used_layers = set()
    for r in rules:
        if r["layers"] != "all":
            used_layers.update(r["layers"])
    layers_sorted = sorted(used_layers)

    bg_rules = sorted(
        (r for r in rules if r["priority"] == 0),
        key=lambda r: (r["priority"], rules.index(r)),
    )

    def rules_for_layer(layer):
        out = []
        for r in rules:
            if r["priority"] == 0:
                continue
            if r["layers"] == "all" or layer in r["layers"]:
                out.append(r)
        out.sort(key=lambda r: (r["priority"], rules.index(r)))
        return out

    universal_rules = sorted(
        (r for r in rules if r["priority"] > 0 and r["layers"] == "all"),
        key=lambda r: (r["priority"], rules.index(r)),
    )

    lines = []
    L = lines.append
    L("#pragma once")
    L("// AUTO-GENERATED from borne_status.toml.")
    L("// Do not edit by hand - run scripts/borne_status_gen.py to regenerate.")
    L("//")
    L("// included once from rgb_matrix_user.inc inside the")
    L("// RGB_MATRIX_CUSTOM_EFFECT_IMPLS guard so symbols stay file-scoped.")
    L("")
    L('#include "quantum.h"')
    L("")
    L("static uint8_t borne_breath_v(void) {")
    L("    return scale8(")
    L("        abs8(sin8(scale16by8(g_rgb_timer, rgb_matrix_config.speed / 8)) - 128) * 2,")
    L("        rgb_matrix_config.hsv.v);")
    L("}")
    L("")
    L("// chart-encoded modifier blend: shift->R, ctrl->G, alt->B,")
    L("// gui bumps base 0x00->0x55 and high 0xAA->0xFF.")
    L("// returns RGB(0,0,0) when no mods are held - this is how the spec's")
    L('// "home row black unless pressed" baseline is implemented.')
    L("static void borne_compute_mod_blend(uint8_t br, bool pulsate,")
    L("                                    uint8_t *out_r, uint8_t *out_g, uint8_t *out_b) {")
    L("    uint8_t mods = get_mods() | get_weak_mods() | get_oneshot_mods();")
    L("    bool sft = mods & MOD_MASK_SHIFT;")
    L("    bool alt = mods & MOD_MASK_ALT;")
    L("    bool ctl = mods & MOD_MASK_CTRL;")
    L("    bool gui = mods & MOD_MASK_GUI;")
    L("    uint8_t base = gui ? 0x55 : 0x00;")
    L("    uint8_t high = gui ? 0xFF : 0xAA;")
    L("    uint8_t r = sft ? high : base;")
    L("    uint8_t g = ctl ? high : base;")
    L("    uint8_t b = alt ? high : base;")
    L("    if (pulsate) { r = scale8(r, br); g = scale8(g, br); b = scale8(b, br); }")
    L("    *out_r = r; *out_g = g; *out_b = b;")
    L("}")
    L("")
    L("static inline void borne_set(uint8_t led, uint8_t led_min, uint8_t led_max,")
    L("                              uint8_t r, uint8_t g, uint8_t b) {")
    L("    if (led >= led_min && led < led_max) rgb_matrix_set_color(led, r, g, b);")
    L("}")
    L("")
    L("static bool borne_status(effect_params_t* params) {")
    L("    RGB_MATRIX_USE_LIMITS(led_min, led_max);")
    L("    uint8_t br = borne_breath_v();")
    L("    uint8_t layer = get_highest_layer(layer_state);")
    L("    led_t leds = host_keyboard_led_state();")
    L("    (void)leds;  // referenced only by gated rules; silence -Wunused if none")
    L("")

    for r in bg_rules:
        L(f"    // priority {r['priority']}: {r['id']}")
        emit_rule(r, groups, colors, lines, "    ")
    L("")
    L("    switch (layer) {")
    for layer in layers_sorted:
        applicable = rules_for_layer(layer)
        if not applicable:
            continue
        L(f"        case {layer}:")
        for r in applicable:
            L(f"            // priority {r['priority']}: {r['id']}")
            emit_rule(r, groups, colors, lines, "            ")
        L("            break;")
    L("        default:")
    for r in universal_rules:
        L(f"            // priority {r['priority']}: {r['id']}")
        emit_rule(r, groups, colors, lines, "            ")
    L("            break;")
    L("    }")
    L("")
    L("    return rgb_matrix_check_finished_leds(led_max);")
    L("}")

    OUT_PATH.write_text("\n".join(lines) + "\n")
    print(f"wrote {OUT_PATH.relative_to(ROOT)} ({len(lines)} lines)")


if __name__ == "__main__":
    main()
