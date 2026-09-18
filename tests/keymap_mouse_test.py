"""Catch mouse-layer keys falling through to their base-layer bindings."""
import re
from pathlib import Path

keymap = (Path(__file__).resolve().parents[1] / "config/roBa.keymap").read_text(encoding="utf-8")
keymap = re.sub(r"//[^\n]*|/\*.*?\*/", "", keymap, flags=re.S)

def bindings(layer):
    match = re.search(r"\b" + re.escape(layer) + r"\s*\{\s*bindings\s*=\s*<(.*?)>;", keymap, re.S)
    assert match, f"Missing {layer} layer"
    return [" ".join(value.split()) for value in re.findall(r"&[^&]+", match[1])]

mouse = bindings("MOUSE")
base = bindings("default_layer")
excluded = set(map(int, re.search(r"excluded-positions\s*=\s*<(.*?)>;", keymap, re.S)[1].split()))
assert len(mouse) == len(base), "Layer lengths do not match"
mouse_positions = {i for i, value in enumerate(mouse)
                   if value.startswith(("&mkp ", "&scroll_mclick "))}
assert {mouse[i].split()[-1] for i in mouse_positions} == {f"MB{i}" for i in range(1, 6)}
assert mouse_positions <= excluded, f"Unprotected mouse buttons: {mouse_positions - excluded}"
assert mouse[19] == "&scroll_mclick 5 MB3" and 19 in excluded, "Middle button must tap MB3 and hold layer 5"
assert base[19] == "&lt 5 K", "Base-layer scrolling must remain available"
hold_tap = re.search(r"scroll_mclick:\s*scroll_mclick\s*\{(.*?)\};", keymap, re.S)[1]
assert re.search(r"bindings\s*=\s*<&mo>\s*,\s*<&mkp>\s*;", hold_tap), "Hold must select a layer; tap must click"
assert 'flavor = "tap-preferred";' in hold_tap
assert 'tapping-term-ms = <200>;' in hold_tap
assert 'quick-tap-ms = <0>;' in hold_tap, "Tap then hold must still activate scroll"
assert 'hold-while-undecided' not in hold_tap and 'retro-tap' not in hold_tap
assert re.search(r"scroll-layers\s*=\s*<5>\s*;", keymap), "Hold must target the driver's scroll layer"
assert '&zip_xy_scaler 5 2' in keymap, "Pointer speed should be 2.5x"
shift_positions = {i for i, value in enumerate(base) if value.startswith("&mt_exit_AML_on_tap LEFT_SHIFT ")}
assert shift_positions <= excluded, "Shift+pointer should preserve the mouse layer"
assert excluded == mouse_positions | shift_positions, "Stale exclusions could block normal typing"
print(f"mouse layer: {len(mouse_positions)} buttons and {len(shift_positions)} Shift keys protected; middle tap / scroll hold configured; pointer 2.5x")
