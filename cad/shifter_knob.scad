/* 
 * Digital Shifter Knob
 * Parametric OpenSCAD Design
 * 
 * This file generates the 3D models for the digital shifter knob,
 * designed to house a 1.28" GC9A01 display and a Seeed XIAO ESP32S3.
 */

// ==========================================
// PARAMETERS
// ==========================================

/* [Global] */
// What part to render
part_to_render = "assembly"; // [assembly, main_body, bezel, cross_section, sensor_bracket]

/* [Outer Shape] */
// Maximum diameter of the knob
knob_outer_diameter = 54;
// Total height of the knob body
knob_height = 60;
// Flat area at the top for the screen
top_flat_diameter = 44;

/* [Display (1.28" GC9A01)] */
// The square PCB dimensions
screen_pcb_size = 40.5; 
screen_pcb_thickness = 1.6;
// The round glass on top of the PCB
screen_glass_diameter = 36.5;
screen_glass_thickness = 1.6;
// The visible screen area (active area is 32.4mm)
bezel_viewing_diameter = 33.5; 
// Depth from the top of the knob to the screen PCB shelf
screen_recess_depth = 6.0;

/* [Internals (XIAO ESP32S3 + Wiring)] */
// Diameter of the internal chamber for the MCU
mcu_cavity_diameter = 26.0;
// Depth of the internal chamber
mcu_cavity_depth = 20.0;

/* [Shaft Mounting] */
// Use standard threaded hole instead of smooth bore with set screw
use_thread = true;
// Standard shift knob thread diameter (e.g. 10 for M10, 12 for M12)
thread_diameter = 12.0;
// Thread pitch (e.g. 1.25 or 1.5)
thread_pitch = 1.25;

// Diameter of the hole for the shifter stalk (if use_thread is false)
shaft_hole_diameter = 14.2;
// Depth of the shaft hole
shaft_hole_depth = 35.0;
// Distance from the bottom for the set screw
set_screw_z_offset = 10.0;
// Diameter of the set screw hole (e.g., 4.2mm for tapping M5)
set_screw_diameter = 4.2;

/* [Hall Sensor Bracket (R53 Mini Cooper S Prototype)] */
// Inner diameter of the ring (wraps around shifter pivot base)
sensor_ring_inner_diam = 40.0;
// Outer diameter of the sensor ring
sensor_ring_outer_diam = 50.0;
// Thickness of the sensor ring
sensor_ring_thickness = 5.0;
// Width of the sensor pocket (for AH49E)
sensor_pocket_width = 4.5;
// Depth of the sensor pocket
sensor_pocket_depth = 2.0;

/* [Tolerances] */
// General clearance for 3D printing fitment
clearance = 0.2;
// Number of fragments for smooth curves
$fn = 100;

// ==========================================
// MODULES
// ==========================================

module internal_thread(d, pitch, length) {
    // Generates a threaded bolt shape to be subtracted
    steps = length / pitch;
    twist_angle = steps * 360;
    
    linear_extrude(height = length, twist = -twist_angle, slices = steps * 40) {
        union() {
            // Core cylinder
            circle(r = d/2 - pitch*0.6, $fn=60);
            // 3-point star for thread teeth (to keep rendering fast while simulating a thread)
            for(i=[0:2])
                rotate(i * 120)
                    translate([d/2 - pitch*0.5, 0])
                        circle(r = pitch * 0.5, $fn=4);
        }
    }
}

module knob_outer_profile() {
    // A classic teardrop / Type-R style knob shape
    // Created by hulling a sphere at the top and a cylinder at the base
    hull() {
        // Top sphere
        translate([0, 0, knob_height - knob_outer_diameter/2])
            sphere(d=knob_outer_diameter);
        // Base cylinder
        cylinder(d=knob_outer_diameter * 0.6, h=5);
    }
}

module screen_cutout() {
    // The top thread/snap ring area for the bezel
    translate([0, 0, knob_height - screen_recess_depth])
        cylinder(d=top_flat_diameter + clearance, h=screen_recess_depth + 1);
    
    // The square shelf for the PCB
    translate([0, 0, knob_height - screen_recess_depth - screen_pcb_thickness])
        cube([screen_pcb_size + clearance, screen_pcb_size + clearance, screen_pcb_thickness * 2], center=true);
        
    // Cutout for the display ribbon cable (usually on one side)
    translate([0, -(screen_pcb_size/2), knob_height - screen_recess_depth - 4])
        cube([15, 10, 8], center=true);
}

module internal_cavities() {
    // Screen and PCB cutout
    screen_cutout();
    
    // MCU and Wiring Cavity
    translate([0, 0, knob_height - screen_recess_depth - screen_pcb_thickness - mcu_cavity_depth])
        cylinder(d=mcu_cavity_diameter, h=mcu_cavity_depth + 1);
        
    // Wire channel connecting MCU cavity to shaft hole (if wires run down the shaft)
    cylinder(d=6.0, h=knob_height);
    
    // Shaft Hole at the bottom
    translate([0, 0, -1]) {
        if (use_thread) {
            // Generate standard M-thread
            internal_thread(d=thread_diameter, pitch=thread_pitch, length=shaft_hole_depth + 2);
            // Small chamfer at entrance to help the thread start
            cylinder(d1=thread_diameter + 1.5, d2=thread_diameter - 1.5, h=3);
        } else {
            cylinder(d=shaft_hole_diameter, h=shaft_hole_depth + 2);
        }
    }
        
    // Set Screw Hole (only if not using threads)
    if (!use_thread) {
        translate([0, 0, set_screw_z_offset])
            rotate([90, 0, 0])
            cylinder(d=set_screw_diameter, h=knob_outer_diameter);
    }
}

module main_body() {
    difference() {
        // Main solid shape
        intersection() {
            knob_outer_profile();
            // Flatten the top for the bezel
            cylinder(d=knob_outer_diameter * 2, h=knob_height);
        }
        
        // Subtract all internals
        internal_cavities();
    }
}

module bezel() {
    bezel_height = screen_recess_depth;
    difference() {
        // Outer bezel shape (matches top flat diameter)
        cylinder(d=top_flat_diameter - clearance, h=bezel_height);
        
        // Viewing window
        translate([0, 0, -1])
            cylinder(d=bezel_viewing_diameter, h=bezel_height + 2);
            
        // Cutout for the screen glass (underside of bezel)
        translate([0, 0, -1])
            cylinder(d=screen_glass_diameter + clearance, h=screen_glass_thickness + 1);
    }
}

module sensor_base_bracket() {
    // A separate parametric bracket to mount at the shifter base (e.g., R53 Mini pivot point)
    difference() {
        // Main ring
        cylinder(d=sensor_ring_outer_diam, h=sensor_ring_thickness);
        
        // Inner hole for shifter base
        translate([0, 0, -1])
            cylinder(d=sensor_ring_inner_diam, h=sensor_ring_thickness + 2);
            
        // Cutout for X-axis Hall Sensor (Front/Back)
        translate([0, (sensor_ring_inner_diam/2), sensor_ring_thickness/2])
            cube([sensor_pocket_width, sensor_pocket_depth * 2, sensor_ring_thickness + 2], center=true);
            
        // Cutout for Y-axis Hall Sensor (Left/Right)
        translate([(sensor_ring_inner_diam/2), 0, sensor_ring_thickness/2])
            cube([sensor_pocket_depth * 2, sensor_pocket_width, sensor_ring_thickness + 2], center=true);
            
        // Small wire channels leading out from the sensors
        translate([0, sensor_ring_outer_diam/2 - 2, sensor_ring_thickness/2])
            cube([2, sensor_ring_outer_diam/2, sensor_ring_thickness + 2], center=true);
        translate([sensor_ring_outer_diam/2 - 2, 0, sensor_ring_thickness/2])
            cube([sensor_ring_outer_diam/2, 2, sensor_ring_thickness + 2], center=true);
    }
}

// ==========================================
// RENDER LOGIC
// ==========================================

if (part_to_render == "assembly") {
    color("DarkSlateGray") main_body();
    translate([0, 0, knob_height - screen_recess_depth])
        color("Silver") bezel();
        
    // Mockup of the screen (for visualization)
    %translate([0, 0, knob_height - screen_recess_depth - screen_pcb_thickness/2])
        cube([screen_pcb_size, screen_pcb_size, screen_pcb_thickness], center=true);
    %translate([0, 0, knob_height - screen_recess_depth])
        cylinder(d=screen_glass_diameter, h=screen_glass_thickness);
} 
else if (part_to_render == "main_body") {
    main_body();
} 
else if (part_to_render == "bezel") {
    // Flip bezel for 3D printing
    rotate([180, 0, 0])
    bezel();
}
else if (part_to_render == "cross_section") {
    difference() {
        union() {
            color("DarkSlateGray") main_body();
            translate([0, 0, knob_height - screen_recess_depth])
                color("Silver") bezel();
        }
        // Slice away half
        translate([-knob_outer_diameter, 0, -10])
            cube([knob_outer_diameter*2, knob_outer_diameter, knob_height + 20]);
    }
}
else if (part_to_render == "sensor_bracket") {
    sensor_base_bracket();
}
