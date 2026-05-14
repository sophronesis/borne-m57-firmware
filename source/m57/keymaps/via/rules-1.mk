VIAL_ENABLE = yes
VIA_ENABLE = yes
ENCODER_MAP_ENABLE = yes
VIAL_INSECURE = yes

VIALRGB_ENABLE = yes

# ============================================================
# VIAL RGB CONTROL (UI ONLY)
# ============================================================
#
# What it DOES:
# - Change RGB mode
# - Adjust brightness, speed, hue
# - Select animations in Vial UI
#
# What it DOES NOT DO:
# - Per-key RGB logic
# - Layer-based lighting behavior
# - Conditional LED logic
#
# Comparison:
#
# Feature                    | QMK RGB Matrix | Vial RGB UI
# ---------------------------|---------------|-------------
# Per-key logic              | ✅ YES        | ❌ NO
# Layer-based behavior       | ✅ YES        | ❌ NO
# Animations                 | ✅ YES        | ✅ YES
# Runtime control            | ❌ NO         | ✅ YES
#
# Summary:
# Vial = control interface
# QMK  = actual RGB logic engine