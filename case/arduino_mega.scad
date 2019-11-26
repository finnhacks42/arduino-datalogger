length = 101.6;
width = 53.3;



module mega_from_svg(){
    plug_h = 11; // height of usb/power components
    h_base = 1.6 + 1.2; // thickness of pcb + space for soldering underneath
    color("blue",0.2)linear_extrude(height = h_base)import (file = "arduino_cad.dxf", layer = "pcb");
    color("green",0.2)translate([0,0,h_base])linear_extrude(height = plug_h)import (file = "arduino_cad.dxf", layer = "usb");
    color("green",0.2)translate([0,0,h_base])linear_extrude(height = plug_h)import (file = "arduino_cad.dxf", layer = "power");
    color("red",0.2)linear_extrude(height = plug_h)import (file = "arduino_cad.dxf", layer = "mounting_holes");
}

module mega(){
    pp = 2; // padding around the plug
    h_pad = 0; // padding above the main body of the arduino
    s_pad = 4; // padding on the left and right
    e_pad = 7; // padding at the end
    m_slack = 1; // slack to leave for mount holes
    
    plug_h = 11; // height of usb/power components
    h_base = 1.6 + 1.2; // thickness of pcb + space for soldering underneath
    header_h = 9;
    m_r = 1.6 - m_slack/2;
    m_h = 5;
    
    translate([e_pad,s_pad,0]){
        // plugs
        translate([91.6,9-pp/2,h_base-pp/2])cube([16.7, 12.3+pp, plug_h+pp]);
        translate([89.5,41.3-pp/2,h_base-pp/2])cube([14.6, 9+pp, plug_h+pp]);

        difference(){
            // base
            translate([-e_pad,-s_pad,0])cube([length+e_pad, width+2*s_pad, h_base+header_h+h_pad]);

            // mounting holes
            union(){
                translate([11.6,2.6,-1])cylinder(m_h+1,m_r,m_r,$fn=20);
                translate([86.4,2.6,-1])cylinder(m_h+1,m_r,m_r,$fn=20);
                translate([87.6,50.5,-1])cylinder(m_h+1,m_r,m_r,$fn=20);
                translate([5.2,50.5,-1])cylinder(m_h+1,m_r,m_r,$fn=20);
            }
        }
    }
}

mega(2,1,0,0,10);





