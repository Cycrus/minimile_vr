// More information: https://danielupshaw.com/openscad-rounded-corners/

// Set to 0.01 for higher definition curves (renders slower)
$fs = 0.15;

module roundedcube(size = [1, 1, 1], center = false, radius = 0.5, apply_to = "all") {
	// If single value, convert to [x, y, z] vector
	size = (size[0] == undef) ? [size, size, size] : size;

	translate_min = radius;
	translate_xmax = size[0] - radius;
	translate_ymax = size[1] - radius;
	translate_zmax = size[2] - radius;

	diameter = radius * 2;

	obj_translate = (center == false) ?
		[0, 0, 0] : [
			-(size[0] / 2),
			-(size[1] / 2),
			-(size[2] / 2)
		];

	translate(v = obj_translate) {
		hull() {
			for (translate_x = [translate_min, translate_xmax]) {
				x_at = (translate_x == translate_min) ? "min" : "max";
				for (translate_y = [translate_min, translate_ymax]) {
					y_at = (translate_y == translate_min) ? "min" : "max";
					for (translate_z = [translate_min, translate_zmax]) {
						z_at = (translate_z == translate_min) ? "min" : "max";

						translate(v = [translate_x, translate_y, translate_z])
						if (
							(apply_to == "all") ||
							(apply_to == "xmin" && x_at == "min") || (apply_to == "xmax" && x_at == "max") ||
							(apply_to == "ymin" && y_at == "min") || (apply_to == "ymax" && y_at == "max") ||
							(apply_to == "zmin" && z_at == "min") || (apply_to == "zmax" && z_at == "max")
						) {
							sphere(r = radius);
						} else {
							rotate = 
								(apply_to == "xmin" || apply_to == "xmax" || apply_to == "x") ? [0, 90, 0] : (
								(apply_to == "ymin" || apply_to == "ymax" || apply_to == "y") ? [90, 90, 0] :
								[0, 0, 0]
							);
							rotate(a = rotate)
							cylinder(h = diameter, r = radius, center = true);
						}
					}
				}
			}
		}
	}
}

length = 80.0;
width = 50.0;
height = 20.0;
wall_thickness = 1.5;

mount_length = 10.0;
mount_width = 5.0;

loadcell_cutout_z_offset = 17.0;
loadcell_cutout_width = 10.0;
loadcell_cutout_height = 10.0;

usb_y_offset = 0.0;
usb_z_offset = 6.0;
usb_width = 10.0;
usb_height = 7.0;

// Box
module box_shape()
{
    union()
    {
        difference()
        {
            roundedcube([length, width, height], center = true,
                        apply_to = "xy",
                        radius = 1.5);
            translate([0, 0, wall_thickness])
            {
                roundedcube([length - 2 * wall_thickness,
                             width - 2 * wall_thickness,
                             height], center = true,
                             apply_to = "zmin",
                             radius = 1.5);
            }
        }
        for (sx = [-1, 1], sy = [-1, 1]) {
            translate([sx * length / 4, sy * width / 2 - sy * mount_width / 2 , 0])
                cube([mount_length, mount_width, height - wall_thickness*3], center = true);
        }
    }
}

module loadcell_cutout()
{
    translate([length / 2 + 0.1, 0, -height / 2 + loadcell_cutout_z_offset])
    {
        roundedcube([wall_thickness * 4, loadcell_cutout_width, loadcell_cutout_height], apply_to = "x", center = true);
    }
}

module usb_cutout()
{
    translate([-length / 2 - 0.1, usb_y_offset, -height / 2 + usb_z_offset])
    roundedcube([wall_thickness * 4, usb_width, usb_height], center = true);
}

// Box
difference()
{
    box_shape();
    loadcell_cutout();
    usb_cutout();
}

// Top
%translate([0, 0, 9])
{
roundedcube([length - wall_thickness*2.4, width - wall_thickness*2.4
    , 3], center = true,
                        apply_to = "xy",
                        radius = 1.5);
}
