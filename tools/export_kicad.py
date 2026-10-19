#!/usr/bin/env python3
"""Export the three main boards without modifying their KiCad source files."""
import argparse
from pathlib import Path
import subprocess
import xml.etree.ElementTree as ET

ROOT = Path(__file__).resolve().parents[1]
BOARDS = {
    "1.0": ROOT / "Hardware/Version 1.0/VariometerV2",
    "2.0": ROOT / "Hardware/Version 2.0/VariometerV2.2/VariometerV2.2",
    "2.1": ROOT / "Hardware/Version 2.1/VariometerV2.2.1/VariometerV2.2.1",
}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--kicad-cli", default="kicad-cli")
    parser.add_argument("--output", type=Path, default=ROOT / "docs/images")
    args = parser.parse_args()
    version = subprocess.check_output([args.kicad_cli, "version"], text=True).strip()
    major = int(version.split(".")[0])
    if major < 7:
        parser.error("KiCad 7+ is required for SVG export; KiCad 9+ for 3D rendering.")
    args.output.mkdir(parents=True, exist_ok=True)

    def run(*parts):
        subprocess.run([args.kicad_cli, *map(str, parts)], check=True)

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
            run("pcb", "render", "--output", args.output / f"pcb-{revision}-3d.png",
                "--width", "1600", "--height", "1000", "--side", "top",
                "--perspective", "--rotate", "25,0,30", pcb)
        else:
            print(f"{revision}: SVGs exported; install KiCad 9+ for pcb render (3D).")
    print(f"Exported using KiCad {version}")


if __name__ == "__main__":
    main()
