#!/usr/bin/env python3
"""Export the three main boards without modifying their KiCad source files."""
import argparse
import json
import os
from pathlib import Path
import re
import subprocess
import tempfile
import xml.etree.ElementTree as ET

ROOT = Path(__file__).resolve().parents[1]
BOARDS = {
    "1.0": ROOT / "Hardware/Version 1.0/VariometerV2",
    "2.0": ROOT / "Hardware/Version 2.0/VariometerV2.2/VariometerV2.2",
    "2.1": ROOT / "Hardware/Version 2.1/VariometerV2.2.1/VariometerV2.2.1",
}


def resolve_models(pcb, model_dir):
    """Relocate old workstation paths in a render-only copy of the board."""
    local = ROOT / "Hardware/Komponenten/3D_Objects"
    files = {p.name.lower(): p for p in local.iterdir() if p.is_file()}
    files["tps73701dcqr.stp"] = local / "TPS73701DCQRG4.stp"
    files["esp32-s3-wroom-1.stp"] = local / "ESP32-S3-WROOM-1.STEP"
    missing = set()
    source = pcb.read_text()

    def relocate(match):
        original = match.group(1)
        if original.startswith(("${KICAD6_3DMODEL_DIR}/", "${KISYS3DMOD}/")):
            candidate = model_dir / original.split("}/", 1)[1]
        else:
            name = re.split(r"[/\\:]", original)[-1].lower()
            candidate = files.get(name)
        if candidate is None or not candidate.is_file():
            missing.add(original)
            return match.group(0)
        return '(model "' + candidate.as_posix() + '"'

    return re.sub(r'\(model "([^"]+)"', relocate, source), sorted(missing)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--kicad-cli", default="kicad-cli")
    parser.add_argument("--output", type=Path, default=ROOT / "docs/images")
    parser.add_argument("--models", type=Path, default=Path(os.environ.get(
        "KICAD9_3DMODEL_DIR", "/usr/share/kicad/3dmodels")),
        help="Installed KiCad component model directory (for old model paths)")
    args = parser.parse_args()
    version = subprocess.check_output([args.kicad_cli, "version"], text=True).strip()
    major = int(version.split(".")[0])
    if major < 7:
        parser.error("KiCad 7+ is required for SVG export; KiCad 9+ for 3D rendering.")
    args.output.mkdir(parents=True, exist_ok=True)

    def run(*parts):
        subprocess.run([args.kicad_cli, *map(str, parts)], check=True)

    renders = {"kicad_version": version, "source_boards_modified": False,
               "model_resolution": "Render-only copies relocate archived workstation paths to repository and installed KiCad models.",
               "revisions": {}}
    for revision, base in BOARDS.items():
        # with_suffix() would strip the final revision number from dotted names.
        pcb = Path(str(base) + ".kicad_pcb")
        schematic = Path(str(base) + ".kicad_sch")
        for side, layers in [("top", "F.Cu,F.SilkS,Edge.Cuts"),
                             ("bottom", "B.Cu,B.SilkS,Edge.Cuts")]:
            options = ["pcb", "export", "svg", "--layers", layers,
                       "--page-size-mode", "2", "--exclude-drawing-sheet",
                       "--output", args.output / f"pcb-{revision}-{side}.svg"]
            if major >= 9:
                options += ["--mode-single"]
            if side == "bottom":
                options += ["--mirror"]
            run(*options, pcb)
            # KiCad's PCB colors assume a dark canvas; SVG exports are otherwise
            # transparent and lose silkscreen contrast on a light README page.
            svg = args.output / f"pcb-{revision}-{side}.svg"
            ET.register_namespace("", "http://www.w3.org/2000/svg")
            drawing = ET.parse(svg)
            drawing.getroot().insert(0, ET.Element(
                "{http://www.w3.org/2000/svg}rect",
                {"width": "100%", "height": "100%", "fill": "#111820"}))
            drawing.write(svg, encoding="utf-8", xml_declaration=True)
        schematic_output = args.output / f"schematic-{revision}"
        schematic_output.mkdir(exist_ok=True)
        run("sch", "export", "svg", "--output", str(schematic_output) + "/", schematic)
        if major >= 9:
            source, missing = resolve_models(pcb, args.models)
            with tempfile.TemporaryDirectory(prefix="variometer-render-") as tmp:
                render_board = Path(tmp) / pcb.name
                render_board.write_text(source)
                run("pcb", "render", "--output", args.output / f"pcb-{revision}-3d.png",
                    "--width", "1600", "--height", "1000", "--side", "top",
                    "--background", "opaque", "--quality", "high",
                    "--preset", "follow_pcb_editor", "--zoom", "0.72",
                    "--perspective", "--rotate", "25,0,30", render_board)
            renders["revisions"][revision] = {"unresolved_model_references": missing}
        else:
            print(f"{revision}: SVGs exported; install KiCad 9+ for pcb render (3D).")
    for svg in args.output.rglob("*.svg"):
        svg.write_text("\n".join(line.rstrip() for line in svg.read_text().splitlines()) + "\n")
    (args.output / "pcb-render-manifest.json").write_text(json.dumps(renders, indent=2) + "\n")
    print(f"Exported using KiCad {version}")


if __name__ == "__main__":
    main()
