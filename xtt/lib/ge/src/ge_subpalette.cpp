/* 
 * Proview   $Id: ge_subpalette.cpp,v 1.13 2008-11-28 17:13:45 claes Exp $
 * Copyright (C) 2005 SSAB Oxel�sund AB.
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License as 
 * published by the Free Software Foundation, either version 2 of 
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with the program, if not, write to the Free Software 
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 **/

#include "glow_std.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <vector>

#include "co_cdh.h"
#include "co_time.h"
#include "pwr_baseclasses.h"
#include "co_dcli.h"
#include "flow.h"
#include "flow_browctx.h"
#include "flow_browapi.h"
#include "glow.h"
#include "glow_growctx.h"
#include "glow_growapi.h"
#include "ge_attr.h"
#include "ge_subpalette.h"

#include "xnav_bitmap_leaf12.h"
#include "xnav_bitmap_map12.h"
#include "xnav_bitmap_openmap12.h"
#include "ge_bitmap_valve1.h"
#include "ge_bitmap_valve2.h"
#include "ge_bitmap_pump3.h"
#include "ge_bitmap_valve4.h"
#include "ge_bitmap_ind5.h"
#include "ge_bitmap_ind6.h"
#include "ge_bitmap_pushbutton7.h"
#include "ge_bitmap_frame8.h"
#include "ge_bitmap_crosscon9.h"
#include "ge_bitmap_threewaycon10.h"
#include "ge_bitmap_cornercon11.h"
#include "ge_bitmap_arrow12.h"
#include "ge_bitmap_checkvalve13.h"
#include "ge_bitmap_tank14.h"
#include "ge_bitmap_filter15.h"
#include "ge_bitmap_pressureswitch16.h"
#include "ge_bitmap_hydrpump17.h"
#include "ge_bitmap_pressuregauge18.h"
#include "ge_bitmap_releasevalve19.h"
#include "ge_bitmap_releasevalve20.h"
#include "ge_bitmap_frame21.h"
#include "ge_bitmap_pincon22.h"
#include "ge_bitmap_motor23.h"
#include "ge_bitmap_speedswitch24.h"
#include "ge_bitmap_trend25.h"
#include "ge_bitmap_bar26.h"
#include "ge_bitmap_axis27.h"
#include "ge_bitmap_thermometer28.h"
#include "ge_bitmap_window29.h"
#include "ge_bitmap_table30.h"
#include "ge_bitmap_optionsmenu31.h"
#include "ge_bitmap_menubar32.h"
#include "ge_bitmap_pulldownmenu33.h"
#include "ge_bitmap_emergencystop34.h"
#include "ge_bitmap_tabbedwindow35.h"
#include "ge_bitmap_trafficlight36.h"
#include "ge_bitmap_slider37.h"
#include "ge_bitmap_sliderbg38.h"
#include "ge_bitmap_padlock39.h"
#include "ge_bitmap_needle40.h"
#include "ge_bitmap_videocamera41.h"
#include "ge_bitmap_terminal42.h"
#include "ge_bitmap_rack43.h"
#include "ge_bitmap_pc44.h"
#include "ge_bitmap_pulpet45.h"
#include "ge_bitmap_screw46.h"
#include "ge_bitmap_ind47.h"
#include "ge_bitmap_ind48.h"
#include "ge_bitmap_value49.h"
#include "ge_bitmap_value50.h"
#include "ge_bitmap_value51.h"
#include "ge_bitmap_value52.h"
#include "ge_bitmap_valve53.h"
#include "ge_bitmap_directvalve54.h"
#include "ge_bitmap_mvalve55.h"
#include "ge_bitmap_basevalve56.h"
#include "ge_bitmap_actuator57.h"
#include "ge_bitmap_handwheel58.h"
#include "ge_bitmap_burner59.h"
#include "ge_bitmap_contactor60.h"
#include "ge_bitmap_contactor61.h"
#include "ge_bitmap_controlswitch62.h"
#include "ge_bitmap_fuse63.h"
#include "ge_bitmap_fuse64.h"
#include "ge_bitmap_motor65.h"
#include "ge_bitmap_overloadrelay66.h"
#include "ge_bitmap_rod67.h"
#include "ge_bitmap_rodcoupling68.h"
#include "ge_bitmap_fan69.h"
#include "ge_bitmap_fan70.h"
#include "ge_bitmap_fan71.h"
#include "ge_bitmap_pump72.h"
#include "ge_bitmap_pump73.h"
#include "ge_bitmap_manswitch74.h"
#include "ge_bitmap_manswitch75.h"
#include "ge_bitmap_manswitch76.h"
#include "ge_bitmap_frequencyconv77.h"
#include "ge_bitmap_slider78.h"
#include "ge_bitmap_smiley79.h"
#include "ge_bitmap_sliderbg80.h"
#include "ge_bitmap_slider81.h"
#include "ge_bitmap_sliderbg82.h"
#include "ge_bitmap_button83.h"
#include "ge_bitmap_button84.h"
#include "ge_bitmap_button85.h"
#include "ge_bitmap_button86.h"
#include "ge_bitmap_button87.h"
#include "ge_bitmap_checkbox88.h"
#include "ge_bitmap_radiobutton89.h"
#include "ge_bitmap_radiobutton90.h"
#include "ge_bitmap_checkbox91.h"
#include "ge_bitmap_button92.h"
#include "ge_bitmap_buttoninfo93.h"
#include "ge_bitmap_buttonhelp94.h"
#include "ge_bitmap_button95.h"
#include "ge_bitmap_notebutton96.h"
#include "ge_bitmap_pump97.h"
#include "ge_bitmap_damper98.h"
#include "ge_bitmap_3wayvalve99.h"
#include "ge_bitmap_aircooler100.h"
#include "ge_bitmap_airheater101.h"
#include "ge_bitmap_elheater102.h"
#include "ge_bitmap_3wayvalvecontrol103.h"
#include "ge_bitmap_dampercontrol104.h"
#include "ge_bitmap_4wayvalve105.h"
#include "ge_bitmap_actuator106.h"
#include "ge_bitmap_actuatordia107.h"
#include "ge_bitmap_actuatorfailclose108.h"
#include "ge_bitmap_actuatorfailopen109.h"
#include "ge_bitmap_actuatormotor110.h"
#include "ge_bitmap_actuatorsensing111.h"
#include "ge_bitmap_actuatorspring112.h"
#include "ge_bitmap_actuatorsolenoid113.h"
#include "ge_bitmap_actuatorpiston114.h"
#include "ge_bitmap_actuatorwithman115.h"
#include "ge_bitmap_aircoolerheater116.h"
#include "ge_bitmap_airfilter117.h"
#include "ge_bitmap_airgrill118.h"
#include "ge_bitmap_airpurgingdevice119.h"
#include "ge_bitmap_auxiliaryunitman120.h"
#include "ge_bitmap_drawoffpoint121.h"
#include "ge_bitmap_gully122.h"
#include "ge_bitmap_gullywithtrap123.h"
#include "ge_bitmap_firedamper124.h"
#include "ge_bitmap_heatingexchanger125.h"
#include "ge_bitmap_heatingexchanger126.h"
#include "ge_bitmap_humidifier127.h"
#include "ge_bitmap_hydrant128.h"
#include "ge_bitmap_noreturnvalve129.h"
#include "ge_bitmap_pressuresource130.h"
#include "ge_bitmap_pump131.h"
#include "ge_bitmap_recorder132.h"
#include "ge_bitmap_restrictionunit133.h"
#include "ge_bitmap_safetyvalve134.h"
#include "ge_bitmap_separator135.h"
#include "ge_bitmap_shower136.h"
#include "ge_bitmap_silencer137.h"
#include "ge_bitmap_sprinklerhead138.h"
#include "ge_bitmap_steamtrap139.h"
#include "ge_bitmap_strainer140.h"
#include "ge_bitmap_thermometer141.h"
#include "ge_bitmap_transmitter142.h"
#include "ge_bitmap_trap143.h"
#include "ge_bitmap_vacuumbreaker144.h"
#include "ge_bitmap_valve145.h"
#include "ge_bitmap_valveel146.h"
#include "ge_bitmap_valvepiston147.h"
#include "ge_bitmap_valvesensing148.h"
#include "ge_bitmap_valvesolenoid149.h"
#include "ge_bitmap_aircooledcondenser150.h"
#include "ge_bitmap_compressor151.h"
#include "ge_bitmap_compressor2step152.h"
#include "ge_bitmap_compressor2stepaggr153.h"
#include "ge_bitmap_compressoraggr154.h"
#include "ge_bitmap_coolingmediumtank155.h"
#include "ge_bitmap_coolingtower156.h"
#include "ge_bitmap_dryingfilter157.h"
#include "ge_bitmap_evaporativecondenser158.h"
#include "ge_bitmap_evaporator159.h"
#include "ge_bitmap_fan160.h"
#include "ge_bitmap_fancondenser161.h"
#include "ge_bitmap_fluidcooledcondenser162.h"
#include "ge_bitmap_sightglass163.h"
#include "ge_bitmap_2waycontact164.h"
#include "ge_bitmap_breakcontact165.h"
#include "ge_bitmap_breakcontactdelayedclose166.h"
#include "ge_bitmap_breakcontactdelayedopen167.h"
#include "ge_bitmap_makecontactdelayedclose168.h"
#include "ge_bitmap_makecontactdelayedopen169.h"
#include "ge_bitmap_chassis170.h"
#include "ge_bitmap_earth171.h"
#include "ge_bitmap_equipotentiality172.h"
#include "ge_bitmap_noiselessearth173.h"
#include "ge_bitmap_protectiveearth174.h"
#include "ge_bitmap_accumulator175.h"
#include "ge_bitmap_capacitor176.h"
#include "ge_bitmap_diode177.h"
#include "ge_bitmap_led178.h"
#include "ge_bitmap_resistor179.h"
#include "ge_bitmap_triodthyristor180.h"
#include "ge_bitmap_changeoverbreakcontact181.h"
#include "ge_bitmap_makecontact182.h"
#include "ge_bitmap_adder183.h"
#include "ge_bitmap_and184.h"
#include "ge_bitmap_controller185.h"
#include "ge_bitmap_delay186.h"
#include "ge_bitmap_division187.h"
#include "ge_bitmap_exclusiveor188.h"
#include "ge_bitmap_filter189.h"
#include "ge_bitmap_max190.h"
#include "ge_bitmap_min191.h"
#include "ge_bitmap_multiplication192.h"
#include "ge_bitmap_negator193.h"
#include "ge_bitmap_or194.h"
#include "ge_bitmap_pulse195.h"
#include "ge_bitmap_rampdown196.h"
#include "ge_bitmap_rampup197.h"
#include "ge_bitmap_root198.h"
#include "ge_bitmap_rs199.h"
#include "ge_bitmap_circuitbreakerauto200.h"
#include "ge_bitmap_fuse201.h"
#include "ge_bitmap_fusedisconnector202.h"
#include "ge_bitmap_fusesupplyside203.h"
#include "ge_bitmap_fuseswitch204.h"
#include "ge_bitmap_fuseswitchdisconnector205.h"
#include "ge_bitmap_converter206.h"
#include "ge_bitmap_converter_ac_ac207.h"
#include "ge_bitmap_converter_ac_dc208.h"
#include "ge_bitmap_converter_dc_ac209.h"
#include "ge_bitmap_converter_dc_dc210.h"
#include "ge_bitmap_machine211.h"
#include "ge_bitmap_machineac212.h"
#include "ge_bitmap_machinedc213.h"
#include "ge_bitmap_bell214.h"
#include "ge_bitmap_horn215.h"
#include "ge_bitmap_lamp216.h"
#include "ge_bitmap_siren217.h"
#include "ge_bitmap_auxcontactthermal218.h"
#include "ge_bitmap_breakcontactthermal219.h"
#include "ge_bitmap_circuitbreaker220.h"
#include "ge_bitmap_circuitbreakerrem221.h"
#include "ge_bitmap_circuitbreakerremfuse222.h"
#include "ge_bitmap_disconnector223.h"
#include "ge_bitmap_emergencycontrolswitch224.h"
#include "ge_bitmap_manswitchbreak225.h"
#include "ge_bitmap_manswitchbreaknoret226.h"
#include "ge_bitmap_manswitchmake227.h"
#include "ge_bitmap_manswitchmakenoret228.h"
#include "ge_bitmap_mlimitswitchbreak229.h"
#include "ge_bitmap_mlimitswitchmake230.h"
#include "ge_bitmap_photocell231.h"
#include "ge_bitmap_proximityswitchbreak232.h"
#include "ge_bitmap_proximityswitchmake233.h"
#include "ge_bitmap_switchdisconnector234.h"
#include "ge_bitmap_currenttransformer235.h"
#include "ge_bitmap_currenttransformer236.h"
#include "ge_bitmap_voltagetransformer237.h"
#include "ge_bitmap_voltagetransformer238.h"
#include "ge_bitmap_voltagetransformer2wnd239.h"
#include "ge_bitmap_actuatorfailkeep240.h"
#include "ge_bitmap_minicircuitbreaker241.h"
#include "ge_bitmap_minicircuitbreaker242.h"
#include "ge_bitmap_minicircuitbreaker243.h"
#include "ge_bitmap_overloadrelay244.h"
#include "ge_bitmap_damper245.h"
#include "ge_bitmap_threewayvalve246.h"
#include "ge_bitmap_elheater247.h"

#define SUBPALETTE__INPUT_SYNTAX 2
#define SUBPALETTE__OBJNOTFOUND 4
#define SUBPALETTE__STRINGTOLONG 6
#define SUBPALETTE__ITEM_NOCREA 8
#define SUBPALETTE__SUCCESS 1

static char null_str[] = "";

class LocalFile {
 public:
  pwr_tFileName name;
};

void subpalette_sort( vector<LocalFile>& fvect)
{
  for ( int i = fvect.size() - 1; i > 0; i--) {
    for ( int j = 0; j < i; j++) {
      if ( strcmp( fvect[i].name, fvect[j].name) < 0) {
	LocalFile tmp = fvect[i];
	fvect[i] = fvect[j];
	fvect[j] = tmp;
      }
    }
  }
}

//
// Convert attribute string to value
//
void SubPalette::message( char sev, char *text)
{
  (message_cb)( parent_ctx, sev, text);
}

//
//  Free pixmaps
//
void SubPaletteBrow::free_pixmaps()
{
  brow_FreeAnnotPixmap( ctx, pixmap_leaf);
  brow_FreeAnnotPixmap( ctx, pixmap_map);
  brow_FreeAnnotPixmap( ctx, pixmap_openmap);
  for ( int i = 0; i < SUBP_PIXMAPS_SIZE - 1; i++) {
    if ( pixmaps[i])
      brow_FreeAnnotPixmap( ctx, pixmaps[i]);
  }
}

//
//  Create pixmaps for leaf, closed map and open map
//

#define ALLOC_PIXMAP( bitmap, idx) \
  for ( i = 0; i < 9; i++) { \
    pixmap_data[i].width = bitmap ## _width; \
    pixmap_data[i].height = bitmap ## _height; \
    pixmap_data[i].bits = (char *) bitmap ## _bits; \
  } \
  if ( idx-1 >= SUBP_PIXMAPS_SIZE) \
    return; \
  brow_AllocAnnotPixmap( ctx, &pixmap_data, &pixmaps[idx-1]);

void SubPaletteBrow::allocate_pixmaps()
{
  flow_sPixmapData pixmap_data;
  int i;

  for ( i = 0; i < 9; i++) {
    pixmap_data[i].width =xnav_bitmap_leaf12_width;
    pixmap_data[i].height =xnav_bitmap_leaf12_height;
    pixmap_data[i].bits =xnav_bitmap_leaf12_bits;
  }

  brow_AllocAnnotPixmap( ctx, &pixmap_data, &pixmap_leaf);

  for ( i = 0; i < 9; i++) {
    pixmap_data[i].width =xnav_bitmap_map12_width;
    pixmap_data[i].height =xnav_bitmap_map12_height;
    pixmap_data[i].bits =xnav_bitmap_map12_bits;
  }

  brow_AllocAnnotPixmap( ctx, &pixmap_data, &pixmap_map);

  for ( i = 0; i < 9; i++) {
    pixmap_data[i].width =xnav_bitmap_openmap12_width;
    pixmap_data[i].height =xnav_bitmap_openmap12_height;
    pixmap_data[i].bits =xnav_bitmap_openmap12_bits;
  }
  brow_AllocAnnotPixmap( ctx, &pixmap_data, &pixmap_openmap);

  ALLOC_PIXMAP( ge_bitmap_valve1, 1);
  ALLOC_PIXMAP( ge_bitmap_valve2, 2);
  ALLOC_PIXMAP( ge_bitmap_pump3, 3);
  ALLOC_PIXMAP( ge_bitmap_valve4, 4);
  ALLOC_PIXMAP( ge_bitmap_ind5, 5);
  ALLOC_PIXMAP( ge_bitmap_ind6, 6);
  ALLOC_PIXMAP( ge_bitmap_pushbutton7, 7);
  ALLOC_PIXMAP( ge_bitmap_frame8, 8);
  ALLOC_PIXMAP( ge_bitmap_crosscon9, 9);
  ALLOC_PIXMAP( ge_bitmap_threewaycon10, 10);
  ALLOC_PIXMAP( ge_bitmap_cornercon11, 11);
  ALLOC_PIXMAP( ge_bitmap_arrow12, 12);
  ALLOC_PIXMAP( ge_bitmap_checkvalve13, 13);
  ALLOC_PIXMAP( ge_bitmap_tank14, 14);
  ALLOC_PIXMAP( ge_bitmap_filter15, 15);
  ALLOC_PIXMAP( ge_bitmap_pressureswitch16, 16);
  ALLOC_PIXMAP( ge_bitmap_hydrpump17, 17);
  ALLOC_PIXMAP( ge_bitmap_pressuregauge18, 18);
  ALLOC_PIXMAP( ge_bitmap_releasevalve19, 19);
  ALLOC_PIXMAP( ge_bitmap_releasevalve20, 20);
  ALLOC_PIXMAP( ge_bitmap_frame21, 21);
  ALLOC_PIXMAP( ge_bitmap_pincon22, 22);
  ALLOC_PIXMAP( ge_bitmap_motor23, 23);
  ALLOC_PIXMAP( ge_bitmap_speedswitch24, 24);
  ALLOC_PIXMAP( ge_bitmap_trend25, 25);
  ALLOC_PIXMAP( ge_bitmap_bar26, 26);
  ALLOC_PIXMAP( ge_bitmap_axis27, 27);
  ALLOC_PIXMAP( ge_bitmap_thermometer28, 28);
  ALLOC_PIXMAP( ge_bitmap_window29, 29);
  ALLOC_PIXMAP( ge_bitmap_table30, 30);
  ALLOC_PIXMAP( ge_bitmap_optionsmenu31, 31);
  ALLOC_PIXMAP( ge_bitmap_menubar32, 32);
  ALLOC_PIXMAP( ge_bitmap_pulldownmenu33, 33);
  ALLOC_PIXMAP( ge_bitmap_emergencystop34, 34);
  ALLOC_PIXMAP( ge_bitmap_tabbedwindow35, 35);
  ALLOC_PIXMAP( ge_bitmap_trafficlight36, 36);
  ALLOC_PIXMAP( ge_bitmap_slider37, 37);
  ALLOC_PIXMAP( ge_bitmap_sliderbg38, 38);
  ALLOC_PIXMAP( ge_bitmap_padlock39, 39);
  ALLOC_PIXMAP( ge_bitmap_needle40, 40);
  ALLOC_PIXMAP( ge_bitmap_videocamera41, 41);
  ALLOC_PIXMAP( ge_bitmap_terminal42, 42);
  ALLOC_PIXMAP( ge_bitmap_rack43, 43);
  ALLOC_PIXMAP( ge_bitmap_pc44, 44);
  ALLOC_PIXMAP( ge_bitmap_pulpet45, 45);
  ALLOC_PIXMAP( ge_bitmap_screw46, 46);
  ALLOC_PIXMAP( ge_bitmap_ind47, 47);
  ALLOC_PIXMAP( ge_bitmap_ind48, 48);
  ALLOC_PIXMAP( ge_bitmap_value49, 49);
  ALLOC_PIXMAP( ge_bitmap_value50, 50);
  ALLOC_PIXMAP( ge_bitmap_value51, 51);
  ALLOC_PIXMAP( ge_bitmap_value52, 52);
  ALLOC_PIXMAP( ge_bitmap_valve53, 53);
  ALLOC_PIXMAP( ge_bitmap_directvalve54, 54);
  ALLOC_PIXMAP( ge_bitmap_mvalve55, 55);
  ALLOC_PIXMAP( ge_bitmap_basevalve56, 56);
  ALLOC_PIXMAP( ge_bitmap_actuator57, 57);
  ALLOC_PIXMAP( ge_bitmap_handwheel58, 58);
  ALLOC_PIXMAP( ge_bitmap_burner59, 59);
  ALLOC_PIXMAP( ge_bitmap_contactor60, 60);
  ALLOC_PIXMAP( ge_bitmap_contactor61, 61);
  ALLOC_PIXMAP( ge_bitmap_controlswitch62, 62);
  ALLOC_PIXMAP( ge_bitmap_fuse63, 63);
  ALLOC_PIXMAP( ge_bitmap_fuse64, 64);
  ALLOC_PIXMAP( ge_bitmap_motor65, 65);
  ALLOC_PIXMAP( ge_bitmap_overloadrelay66, 66);
  ALLOC_PIXMAP( ge_bitmap_rod67, 67);
  ALLOC_PIXMAP( ge_bitmap_rodcoupling68, 68);
  ALLOC_PIXMAP( ge_bitmap_fan69, 69);
  ALLOC_PIXMAP( ge_bitmap_fan70, 70);
  ALLOC_PIXMAP( ge_bitmap_fan71, 71);
  ALLOC_PIXMAP( ge_bitmap_pump72, 72);
  ALLOC_PIXMAP( ge_bitmap_pump73, 73);
  ALLOC_PIXMAP( ge_bitmap_manswitch74, 74);
  ALLOC_PIXMAP( ge_bitmap_manswitch75, 75);
  ALLOC_PIXMAP( ge_bitmap_manswitch76, 76);
  ALLOC_PIXMAP( ge_bitmap_frequencyconv77, 77);
  ALLOC_PIXMAP( ge_bitmap_slider78, 78);
  ALLOC_PIXMAP( ge_bitmap_smiley79, 79);
  ALLOC_PIXMAP( ge_bitmap_sliderbg80, 80);
  ALLOC_PIXMAP( ge_bitmap_slider81, 81);
  ALLOC_PIXMAP( ge_bitmap_sliderbg82, 82);
  ALLOC_PIXMAP( ge_bitmap_button83, 83);
  ALLOC_PIXMAP( ge_bitmap_button84, 84);
  ALLOC_PIXMAP( ge_bitmap_button85, 85);
  ALLOC_PIXMAP( ge_bitmap_button86, 86);
  ALLOC_PIXMAP( ge_bitmap_button87, 87);
  ALLOC_PIXMAP( ge_bitmap_checkbox88, 88);
  ALLOC_PIXMAP( ge_bitmap_radiobutton89, 89);
  ALLOC_PIXMAP( ge_bitmap_radiobutton90, 90);
  ALLOC_PIXMAP( ge_bitmap_checkbox91, 91);
  ALLOC_PIXMAP( ge_bitmap_button92, 92);
  ALLOC_PIXMAP( ge_bitmap_buttoninfo93, 93);
  ALLOC_PIXMAP( ge_bitmap_buttonhelp94, 94);
  ALLOC_PIXMAP( ge_bitmap_button95, 95);
  ALLOC_PIXMAP( ge_bitmap_notebutton96, 96);
  ALLOC_PIXMAP( ge_bitmap_pump97, 97);
  ALLOC_PIXMAP( ge_bitmap_damper98, 98);
  ALLOC_PIXMAP( ge_bitmap_3wayvalve99, 99);
  ALLOC_PIXMAP( ge_bitmap_aircooler100, 100);
  ALLOC_PIXMAP( ge_bitmap_airheater101, 101);
  ALLOC_PIXMAP( ge_bitmap_elheater102, 102);
  ALLOC_PIXMAP( ge_bitmap_3wayvalvecontrol103, 103);
  ALLOC_PIXMAP( ge_bitmap_dampercontrol104, 104);
  ALLOC_PIXMAP( ge_bitmap_4wayvalve105, 105);
  ALLOC_PIXMAP( ge_bitmap_actuator106, 106);
  ALLOC_PIXMAP( ge_bitmap_actuatordia107, 107);
  ALLOC_PIXMAP( ge_bitmap_actuatorfailclose108, 108);
  ALLOC_PIXMAP( ge_bitmap_actuatorfailopen109, 109);
  ALLOC_PIXMAP( ge_bitmap_actuatormotor110, 110);
  ALLOC_PIXMAP( ge_bitmap_actuatorsensing111, 111);
  ALLOC_PIXMAP( ge_bitmap_actuatorspring112, 112);
  ALLOC_PIXMAP( ge_bitmap_actuatorsolenoid113, 113);
  ALLOC_PIXMAP( ge_bitmap_actuatorpiston114, 114);
  ALLOC_PIXMAP( ge_bitmap_actuatorwithman115, 115);
  ALLOC_PIXMAP( ge_bitmap_aircoolerheater116, 116);
  ALLOC_PIXMAP( ge_bitmap_airfilter117, 117);
  ALLOC_PIXMAP( ge_bitmap_airgrill118, 118);
  ALLOC_PIXMAP( ge_bitmap_airpurgingdevice119, 119);
  ALLOC_PIXMAP( ge_bitmap_auxiliaryunitman120, 120);
  ALLOC_PIXMAP( ge_bitmap_drawoffpoint121, 121);
  ALLOC_PIXMAP( ge_bitmap_gully122, 122);
  ALLOC_PIXMAP( ge_bitmap_gullywithtrap123, 123);
  ALLOC_PIXMAP( ge_bitmap_firedamper124, 124);
  ALLOC_PIXMAP( ge_bitmap_heatingexchanger125, 125);
  ALLOC_PIXMAP( ge_bitmap_heatingexchanger126, 126);
  ALLOC_PIXMAP( ge_bitmap_humidifier127, 127);
  ALLOC_PIXMAP( ge_bitmap_hydrant128, 128);
  ALLOC_PIXMAP( ge_bitmap_noreturnvalve129, 129);
  ALLOC_PIXMAP( ge_bitmap_pressuresource130, 130);
  ALLOC_PIXMAP( ge_bitmap_pump131, 131);
  ALLOC_PIXMAP( ge_bitmap_recorder132, 132);
  ALLOC_PIXMAP( ge_bitmap_restrictionunit133, 133);
  ALLOC_PIXMAP( ge_bitmap_safetyvalve134, 134);
  ALLOC_PIXMAP( ge_bitmap_separator135, 135);
  ALLOC_PIXMAP( ge_bitmap_shower136, 136);
  ALLOC_PIXMAP( ge_bitmap_silencer137, 137);
  ALLOC_PIXMAP( ge_bitmap_sprinklerhead138, 138);
  ALLOC_PIXMAP( ge_bitmap_steamtrap139, 139);
  ALLOC_PIXMAP( ge_bitmap_strainer140, 140);
  ALLOC_PIXMAP( ge_bitmap_thermometer141, 141);
  ALLOC_PIXMAP( ge_bitmap_transmitter142, 142);
  ALLOC_PIXMAP( ge_bitmap_trap143, 143);
  ALLOC_PIXMAP( ge_bitmap_vacuumbreaker144, 144);
  ALLOC_PIXMAP( ge_bitmap_valve145, 145);
  ALLOC_PIXMAP( ge_bitmap_valveel146, 146);
  ALLOC_PIXMAP( ge_bitmap_valvepiston147, 147);
  ALLOC_PIXMAP( ge_bitmap_valvesensing148, 148);
  ALLOC_PIXMAP( ge_bitmap_valvesolenoid149, 149);
  ALLOC_PIXMAP( ge_bitmap_aircooledcondenser150, 150);
  ALLOC_PIXMAP( ge_bitmap_compressor151, 151);
  ALLOC_PIXMAP( ge_bitmap_compressor2step152, 152);
  ALLOC_PIXMAP( ge_bitmap_compressor2stepaggr153, 153);
  ALLOC_PIXMAP( ge_bitmap_compressoraggr154, 154);
  ALLOC_PIXMAP( ge_bitmap_coolingmediumtank155, 155);
  ALLOC_PIXMAP( ge_bitmap_coolingtower156, 156);
  ALLOC_PIXMAP( ge_bitmap_dryingfilter157, 157);
  ALLOC_PIXMAP( ge_bitmap_evaporativecondenser158, 158);
  ALLOC_PIXMAP( ge_bitmap_evaporator159, 159);
  ALLOC_PIXMAP( ge_bitmap_fan160, 160);
  ALLOC_PIXMAP( ge_bitmap_fancondenser161, 161);
  ALLOC_PIXMAP( ge_bitmap_fluidcooledcondenser162, 162);
  ALLOC_PIXMAP( ge_bitmap_sightglass163, 163);
  ALLOC_PIXMAP( ge_bitmap_2waycontact164, 164);
  ALLOC_PIXMAP( ge_bitmap_breakcontact165, 165);
  ALLOC_PIXMAP( ge_bitmap_breakcontactdelayedclose166, 166);
  ALLOC_PIXMAP( ge_bitmap_breakcontactdelayedopen167, 167);
  ALLOC_PIXMAP( ge_bitmap_makecontactdelayedclose168, 168);
  ALLOC_PIXMAP( ge_bitmap_makecontactdelayedopen169, 169);
  ALLOC_PIXMAP( ge_bitmap_chassis170, 170);
  ALLOC_PIXMAP( ge_bitmap_earth171, 171);
  ALLOC_PIXMAP( ge_bitmap_equipotentiality172, 172);
  ALLOC_PIXMAP( ge_bitmap_noiselessearth173, 173);
  ALLOC_PIXMAP( ge_bitmap_protectiveearth174, 174);
  ALLOC_PIXMAP( ge_bitmap_accumulator175, 175);
  ALLOC_PIXMAP( ge_bitmap_capacitor176, 176);
  ALLOC_PIXMAP( ge_bitmap_diode177, 177);
  ALLOC_PIXMAP( ge_bitmap_led178, 178);
  ALLOC_PIXMAP( ge_bitmap_resistor179, 179);
  ALLOC_PIXMAP( ge_bitmap_triodthyristor180, 180);
  ALLOC_PIXMAP( ge_bitmap_changeoverbreakcontact181, 181);
  ALLOC_PIXMAP( ge_bitmap_makecontact182, 182);
  ALLOC_PIXMAP( ge_bitmap_adder183, 183);
  ALLOC_PIXMAP( ge_bitmap_and184, 184);
  ALLOC_PIXMAP( ge_bitmap_controller185, 185);
  ALLOC_PIXMAP( ge_bitmap_delay186, 186);
  ALLOC_PIXMAP( ge_bitmap_division187, 187);
  ALLOC_PIXMAP( ge_bitmap_exclusiveor188, 188);
  ALLOC_PIXMAP( ge_bitmap_filter189, 189);
  ALLOC_PIXMAP( ge_bitmap_max190, 190);
  ALLOC_PIXMAP( ge_bitmap_min191, 191);
  ALLOC_PIXMAP( ge_bitmap_multiplication192, 192);
  ALLOC_PIXMAP( ge_bitmap_negator193, 193);
  ALLOC_PIXMAP( ge_bitmap_or194, 194);
  ALLOC_PIXMAP( ge_bitmap_pulse195, 195);
  ALLOC_PIXMAP( ge_bitmap_rampdown196, 196);
  ALLOC_PIXMAP( ge_bitmap_rampup197, 197);
  ALLOC_PIXMAP( ge_bitmap_root198, 198);
  ALLOC_PIXMAP( ge_bitmap_rs199, 199);
  ALLOC_PIXMAP( ge_bitmap_circuitbreakerauto200, 200);
  ALLOC_PIXMAP( ge_bitmap_fuse201, 201);
  ALLOC_PIXMAP( ge_bitmap_fusedisconnector202, 202);
  ALLOC_PIXMAP( ge_bitmap_fusesupplyside203, 203);
  ALLOC_PIXMAP( ge_bitmap_fuseswitch204, 204);
  ALLOC_PIXMAP( ge_bitmap_fuseswitchdisconnector205, 205);
  ALLOC_PIXMAP( ge_bitmap_converter206, 206);
  ALLOC_PIXMAP( ge_bitmap_converter_ac_ac207, 207);
  ALLOC_PIXMAP( ge_bitmap_converter_ac_dc208, 208);
  ALLOC_PIXMAP( ge_bitmap_converter_dc_ac209, 209);
  ALLOC_PIXMAP( ge_bitmap_converter_dc_dc210, 210);
  ALLOC_PIXMAP( ge_bitmap_machine211, 211);
  ALLOC_PIXMAP( ge_bitmap_machineac212, 212);
  ALLOC_PIXMAP( ge_bitmap_machinedc213, 213);
  ALLOC_PIXMAP( ge_bitmap_bell214, 214);
  ALLOC_PIXMAP( ge_bitmap_horn215, 215);
  ALLOC_PIXMAP( ge_bitmap_lamp216, 216);
  ALLOC_PIXMAP( ge_bitmap_siren217, 217);
  ALLOC_PIXMAP( ge_bitmap_auxcontactthermal218, 218);
  ALLOC_PIXMAP( ge_bitmap_breakcontactthermal219, 219);
  ALLOC_PIXMAP( ge_bitmap_circuitbreaker220, 220);
  ALLOC_PIXMAP( ge_bitmap_circuitbreakerrem221, 221);
  ALLOC_PIXMAP( ge_bitmap_circuitbreakerremfuse222, 222);
  ALLOC_PIXMAP( ge_bitmap_disconnector223, 223);
  ALLOC_PIXMAP( ge_bitmap_emergencycontrolswitch224, 224);
  ALLOC_PIXMAP( ge_bitmap_manswitchbreak225, 225);
  ALLOC_PIXMAP( ge_bitmap_manswitchbreaknoret226, 226);
  ALLOC_PIXMAP( ge_bitmap_manswitchmake227, 227);
  ALLOC_PIXMAP( ge_bitmap_manswitchmakenoret228, 228);
  ALLOC_PIXMAP( ge_bitmap_mlimitswitchbreak229, 229);
  ALLOC_PIXMAP( ge_bitmap_mlimitswitchmake230, 230);
  ALLOC_PIXMAP( ge_bitmap_photocell231, 231);
  ALLOC_PIXMAP( ge_bitmap_proximityswitchbreak232, 232);
  ALLOC_PIXMAP( ge_bitmap_proximityswitchmake233, 233);
  ALLOC_PIXMAP( ge_bitmap_switchdisconnector234, 234);
  ALLOC_PIXMAP( ge_bitmap_currenttransformer235, 235);
  ALLOC_PIXMAP( ge_bitmap_currenttransformer236, 236);
  ALLOC_PIXMAP( ge_bitmap_voltagetransformer237, 237);
  ALLOC_PIXMAP( ge_bitmap_voltagetransformer238, 238);
  ALLOC_PIXMAP( ge_bitmap_voltagetransformer2wnd239, 239);
  ALLOC_PIXMAP( ge_bitmap_actuatorfailkeep240, 240);
  ALLOC_PIXMAP( ge_bitmap_minicircuitbreaker241, 241);
  ALLOC_PIXMAP( ge_bitmap_minicircuitbreaker242, 242);
  ALLOC_PIXMAP( ge_bitmap_minicircuitbreaker243, 243);
  ALLOC_PIXMAP( ge_bitmap_overloadrelay244, 244);
  ALLOC_PIXMAP( ge_bitmap_damper245, 245);
  ALLOC_PIXMAP( ge_bitmap_threewayvalve246, 246);
  ALLOC_PIXMAP( ge_bitmap_elheater247, 247);
}


//
// Create the navigator widget
//
SubPalette::SubPalette(
	void *xn_parent_ctx,
	const char *xn_name,
	pwr_tStatus *status) :
	parent_ctx(xn_parent_ctx),
	trace_started(0),
	message_cb(NULL), traverse_focus_cb(NULL), set_focus_cb(NULL),
	help_cb(NULL), menu_tree(NULL), path_cnt(0), displayed(0)
{
  strcpy( name, xn_name);
  *status = 1;
}

//
//  Delete a nav context
//
SubPalette::~SubPalette()
{
}

SubPaletteBrow::~SubPaletteBrow()
{
  free_pixmaps();
}

//
//  Return associated class of selected object
//

int SubPalette::get_select( char *text, char *filename)
{
  brow_tNode	*node_list;
  int		node_count;
  Item		*item;
  int		sts;
  
  brow_GetSelectedNodes( brow->ctx, &node_list, &node_count);
  if ( !node_count)
    return 0;

  brow_GetUserData( node_list[0], (void **)&item);
  free( node_list);

  sts = 0;
  switch( item->type)
  {
    case subpalette_eItemType_File:
      strcpy( filename, ((ItemFile *)item)->filename);
      strcpy( text, ((ItemFile *)item)->name);
      sts = 1;
      break;
    default:
      ;
  }
  return sts;
}


//
// Callbacks from brow
//
static int subpalette_brow_cb( FlowCtx *ctx, flow_tEvent event)
{
  SubPalette		*subpalette;
  ItemLocalSubGraphs 	*item;

  if ( event->event == flow_eEvent_ObjectDeleted)
  {
    brow_GetUserData( event->object.object, (void **)&item);
    delete item;
    return 1;
  }

  brow_GetCtxUserData( (BrowCtx *)ctx, (void **) &subpalette);
  subpalette->message( ' ', null_str);
  switch ( event->event)
  {
    case flow_eEvent_Key_PageDown: {
      brow_Page( subpalette->brow->ctx, 0.9);
      break;
    }
    case flow_eEvent_Key_PageUp: {
      brow_Page( subpalette->brow->ctx, -0.9);
      break;
    }
    case flow_eEvent_ScrollDown: {
      brow_Page( subpalette->brow->ctx, 0.1);
      break;
    }
    case flow_eEvent_ScrollUp: {
      brow_Page( subpalette->brow->ctx, -0.1);
      break;
    }
    case flow_eEvent_Key_Up:
    {
      brow_tNode	*node_list;
      int		node_count;
      brow_tObject	object;
      int		sts;

      brow_GetSelectedNodes( subpalette->brow->ctx, &node_list, &node_count);
      if ( !node_count) {
        sts = brow_GetLastVisible( subpalette->brow->ctx, &object);
        if ( EVEN(sts)) return 1;
      }
      else {
	if ( !brow_IsVisible( subpalette->brow->ctx, node_list[0], flow_eVisible_Partial)) {
	  sts = brow_GetLastVisible( subpalette->brow->ctx, &object);
	  if ( EVEN(sts)) return 1;
	}
	else {
	  sts = brow_GetPrevious( subpalette->brow->ctx, node_list[0], &object);
	  if ( EVEN(sts)) {
            if ( node_count)
	      free( node_list);
            return 1;
 	  }
        }
      }
      brow_SelectClear( subpalette->brow->ctx);
      brow_SetInverse( object, 1);
      brow_SelectInsert( subpalette->brow->ctx, object);
      if ( !brow_IsVisible( subpalette->brow->ctx, object, flow_eVisible_Full))
        brow_CenterObject( subpalette->brow->ctx, object, 0.25);
      if ( node_count)
        free( node_list);
      break;
    }
    case flow_eEvent_Key_Down:
    {
      brow_tNode	*node_list;
      int		node_count;
      brow_tObject	object;
      int		sts;

      brow_GetSelectedNodes( subpalette->brow->ctx, &node_list, &node_count);
      if ( !node_count) {
        sts = brow_GetFirstVisible( subpalette->brow->ctx, &object);
        if ( EVEN(sts)) return 1;
      }
      else {
	if ( !brow_IsVisible( subpalette->brow->ctx, node_list[0], flow_eVisible_Partial)) {
	  sts = brow_GetFirstVisible( subpalette->brow->ctx, &object);
	  if ( EVEN(sts)) return 1;
	}
	else {
	  sts = brow_GetNext( subpalette->brow->ctx, node_list[0], &object);
	  if ( EVEN(sts)) {
            if ( node_count)
	      free( node_list);
            return 1;
 	  }
        }
      }
      brow_SelectClear( subpalette->brow->ctx);
      brow_SetInverse( object, 1);
      brow_SelectInsert( subpalette->brow->ctx, object);
      if ( !brow_IsVisible( subpalette->brow->ctx, object, flow_eVisible_Full))
        brow_CenterObject( subpalette->brow->ctx, object, 0.75);
      if ( node_count)
        free( node_list);
      break;
    }
    case flow_eEvent_SelectClear:
      brow_ResetSelectInverse( subpalette->brow->ctx);
      break;
    case flow_eEvent_MB1Click:
      // Select
      double ll_x, ll_y, ur_x, ur_y;
      int		sts;

      if ( subpalette->set_focus_cb)
        (subpalette->set_focus_cb)( subpalette->parent_ctx, subpalette);

      switch ( event->object.object_type)
      {
        case flow_eObjectType_Node:
          brow_MeasureNode( event->object.object, &ll_x, &ll_y,
			&ur_x, &ur_y);
	  if ( event->object.x < ll_x + 1.0)
          {
            // Simulate doubleclick
            flow_tEvent doubleclick_event;

            doubleclick_event = (flow_tEvent) calloc( 1, sizeof(*doubleclick_event));
            memcpy( doubleclick_event, event, sizeof(*doubleclick_event));
            doubleclick_event->event = flow_eEvent_MB1DoubleClick;
            sts = subpalette_brow_cb( ctx, doubleclick_event);
            free( (char *) doubleclick_event);
            return sts;
          }

          if ( brow_FindSelectedObject( subpalette->brow->ctx, event->object.object))
          {
            brow_SelectClear( subpalette->brow->ctx);
          }
          else
          {
            brow_SelectClear( subpalette->brow->ctx);
            brow_SetInverse( event->object.object, 1);
            brow_SelectInsert( subpalette->brow->ctx, event->object.object);
          }
          break;
        default:
          brow_SelectClear( subpalette->brow->ctx);
      }
      break;
    case flow_eEvent_MB3Press: {            
      switch ( event->object.object_type) {
        case flow_eObjectType_Node:
	  ItemFile 	*item;
          brow_GetUserData( event->object.object, (void **)&item);
          if ( item->type != subpalette_eItemType_File)
            break;

	  subpalette->create_popup_menu( item->filename,
					 event->any.x_pixel, event->any.y_pixel);
          break;
        default:
          ;
      }
      break;
    }
    case flow_eEvent_Key_Return:
    case flow_eEvent_Key_Right:
    {
      brow_tNode	*node_list;
      int		node_count;

      brow_GetSelectedNodes( subpalette->brow->ctx, &node_list, &node_count);
      if ( !node_count)
        break;
      brow_GetUserData( node_list[0], (void **)&item);
      free( node_list);

      item->open_children( subpalette, 0, 0);
      break;
    }
    case flow_eEvent_Key_PF4:
    case flow_eEvent_Key_Left:
    {
      brow_tNode	*node_list;
      int		node_count;
      brow_tObject	object;

      brow_GetSelectedNodes( subpalette->brow->ctx, &node_list, &node_count);
      if ( !node_count)
        break;
      if ( !node_count)
        break;

      if ( brow_IsOpen( node_list[0]))
        // Close this node
        object = node_list[0];
      else {
        // Close parent
        sts = brow_GetParent( subpalette->brow->ctx, node_list[0], &object);
        if ( EVEN(sts)) {
          free( node_list);
          return 1;
        }
      }

      brow_GetUserData( object, (void **)&item);
      item->close( subpalette, 0, 0);

      brow_SelectClear( subpalette->brow->ctx);
      brow_SetInverse( object, 1);
      brow_SelectInsert( subpalette->brow->ctx, object);
      if ( !brow_IsVisible( subpalette->brow->ctx, object, flow_eVisible_Full))
        brow_CenterObject( subpalette->brow->ctx, object, 0.25);
      free( node_list);
      break;
    }
    case flow_eEvent_MB1DoubleClick:
      switch ( event->object.object_type)
      {
        case flow_eObjectType_Node:
          brow_GetUserData( event->object.object, (void **)&item);
	  item->open_children( subpalette, event->object.x, event->object.y);
          break;
        default:
          ;
      }
      break;
    case flow_eEvent_Key_Tab:
    {
      if ( subpalette->traverse_focus_cb)
        (subpalette->traverse_focus_cb)( subpalette->parent_ctx, subpalette);
      break;
    }
    case flow_eEvent_Map:
    {
      subpalette->displayed = 1;
      break;
    }
    default:
      ;
  }
  return 1;
}


//
// Create nodeclasses
//
void SubPaletteBrow::create_nodeclasses()
{
  allocate_pixmaps();

  // Create common-class

  brow_CreateNodeClass( ctx, "NavigatorDefault", 
		flow_eNodeGroup_Common, &nc_object);
  brow_AddAnnotPixmap( nc_object, 0, 0.2, 0.1, flow_eDrawType_Line, 2, 0);
  brow_AddAnnot( nc_object, 1.5, 0.6, 0,
		flow_eDrawType_TextHelvetica, 2, flow_eAnnotType_OneLine, 
		0);
  brow_AddAnnotPixmap( nc_object, 1, 7.8, 0.1, flow_eDrawType_Line, 2, 0);
  brow_AddFrame( nc_object, 0, 0, 20, 0.83, flow_eDrawType_LineGray, -1, 1);

  // Create subgraph-class

  brow_CreateNodeClass( ctx, "NavigatorSubgraph", 
		flow_eNodeGroup_Common, &nc_sub);
  brow_AddAnnotPixmap( nc_sub, 0, 0.2, 0.1, flow_eDrawType_Line, 2, 0);
  brow_AddAnnotPixmap( nc_sub, 1, 1.4, 0.1, flow_eDrawType_Line, 2, 0);
  brow_AddAnnot( nc_sub, 2.7, 0.6, 0,
		flow_eDrawType_TextHelvetica, 2, flow_eAnnotType_OneLine, 
		0);
  brow_AddFrame( nc_sub, 0, 0, 20, 0.83, flow_eDrawType_LineGray, -1, 1);

}

int	SubPalette::object_attr()
{
  char setup_filename[120];

  dcli_translate_filename( setup_filename, "pwr_exe:pwr_ge_setup.dat");

  brow_SetNodraw( brow->ctx);

  menu_tree_build( setup_filename);

  // Create the root item
  root_item = new ItemMenu( this, "Root",
	NULL, flow_eDest_After, &menu_tree, 1);

  // Open the root item
  ((ItemMenu *)root_item)->open_children( this, 0, 0);

  brow_ResetNodraw( brow->ctx);
  brow_Redraw( brow->ctx, 0);
  return SUBPALETTE__SUCCESS;
}

void SubPalette::select_by_name( char *name)
{
  // Refresh
  brow_SetNodraw( brow->ctx);
  brow_DeleteAll( brow->ctx);
  ((ItemMenu *)root_item)->open_children( this, 0, 0);

  char *s = name;
  char *t = name;
  char itemname[200];
  int level = 0;
  brow_tObject *nodelist;
  brow_tObject current;
  brow_tObject child;
  int nodecnt;
  Item *item;
  int sts;

  for (;;) {
    if ( !t)
      break;

    level ++;
    strcpy( itemname, t);
    if ( (s = strchr( itemname, '-'))) {
      *s = 0;
      t += (s - itemname + 1);

    }
    else 
      t = 0;

    if ( level == 1) {
      brow_GetObjectList( brow->ctx, &nodelist, &nodecnt);
      for ( int i = 0; i < nodecnt; i++) {
  
	brow_GetUserData( nodelist[i], (void **)&item);
	if ( strcmp( itemname, item->name) == 0) {
	  current = nodelist[i];
	}
      }
    }
    else {
      current = 0;
      item->open_children( this, 0, 0);
      for ( sts = brow_GetChild( brow->ctx, item->node, &child);
	    ODD( sts);
	    sts = brow_GetNextSibling( brow->ctx, child, &child)) {

	brow_GetUserData( child, (void **)&item);
	if ( cdh_NoCaseStrcmp( itemname, item->name) == 0) {
	  current = child;
	  break;
	}
      }
      if ( !current)
	break;
    }

  }
  brow_ResetNodraw( brow->ctx);
  brow_Redraw( brow->ctx, 0);

  if ( current) {
    brow_SetInverse( current, 1);
    brow_SelectInsert( brow->ctx, current);
    if ( !brow_IsVisible( brow->ctx, current, flow_eVisible_Full))
      brow_CenterObject( brow->ctx, current, 0.25);
  }
}

void SubPalette::menu_tree_build( char *filename)
{
  ifstream	fp;
  int		line_cnt = 0;

  if ( !check_file( filename))
    return;

  fp.open( filename);
#ifndef OS_VMS
  if ( !fp)
    return;
#endif

  line_cnt = 0;
  menu_tree = menu_tree_build_children( &fp, &line_cnt, filename, NULL);
  fp.close();
}

subpalette_sMenu *SubPalette::menu_tree_build_children( ifstream *fp, int *line_cnt,
		char *filename, subpalette_sMenu *parent)
{
  subpalette_sMenu	*menu_p, *prev;
  subpalette_sMenu	*return_menu = NULL;
  int			first = 1;
  int			nr;
  char			line[140];
  char			file[120];
  char			type[120];
  int			pixmap;

  while ( 1)
  {
    fp->getline( line, sizeof( line));
    if ( line[0] == 0)
      break;
    (*line_cnt)++;
    if ( line[0] == '!' || line[0] == '#')
      continue;
    nr = sscanf( line, "%s %s %s %d", type, name, file, &pixmap);
    if ( nr < 1 )
      printf( "** Syntax error in file %s, line %d", filename, *line_cnt);

    if ( strcmp( type, "{") == 0)
    {
      if ( nr != 1 || !menu_p)
        printf( "** Syntax error in file %s, line %d", filename, *line_cnt);
      menu_p->child_list = menu_tree_build_children( fp, line_cnt, filename,
		menu_p);
    }
    else if ( strcmp( type, "}") == 0)
    {
      if ( nr != 1 )
        printf( "** Syntax error in file %s, line %d", filename, *line_cnt);
      return return_menu;
    }
    else if ( strcmp( type, "menu") == 0)
    {
      if ( nr != 2 )
        printf( "** Syntax error in file %s, line %d", filename, *line_cnt);

      menu_p = (subpalette_sMenu *) calloc( 1, sizeof(subpalette_sMenu));        
      menu_p->parent = parent;
      menu_p->item_type = subpalette_eItemType_Menu;
      strcpy( menu_p->title, name);
      if ( first)
      {
        return_menu = menu_p;
        first = 0;
      }
      else
        prev->next = menu_p;
      prev = menu_p;
    }
    else if ( strcmp( type, "localsubgraphs") == 0)
    {
      if ( nr != 3 )
        printf( "** Syntax error in file %s, line %d", filename, *line_cnt);

      menu_p = (subpalette_sMenu *) calloc( 1, sizeof(subpalette_sMenu));        
      menu_p->parent = parent;
      menu_p->item_type = subpalette_eItemType_LocalSubGraphs;
      strcpy( menu_p->title, name);
      strcpy( menu_p->file, file);
      if ( first)
      {
        return_menu = menu_p;
        first = 0;
      }
      else
        prev->next = menu_p;
      prev = menu_p;
    }
    else if ( strcmp( type, "subgraph") == 0)
    {
      if ( nr != 4 )
        printf( "** Syntax error in file %s, line %d", filename, *line_cnt);

      menu_p = (subpalette_sMenu *) calloc( 1, sizeof(subpalette_sMenu));
      menu_p->parent = parent;
      menu_p->item_type = subpalette_eItemType_File;
      strcpy( menu_p->title, name);
      dcli_get_defaultfilename( file, menu_p->file, ".pwsg");
      menu_p->pixmap = pixmap;
      if ( first)
      {
        return_menu = menu_p;
        first = 0;
      }
      else
        prev->next = menu_p;
      prev = menu_p;
    }
    else if ( strcmp( type, "path") == 0)
    {
      if ( nr != 1 )
        printf( "** Syntax error in file %s, line %d", filename, *line_cnt);

      fp->getline( line, sizeof( line));
      if ( line[0] == 0)
        break;
      (*line_cnt)++;
      if ( line[0] == '!' || line[0] == '#')
        continue;
      nr = sscanf( line, "%s %s %s %d", type, name, file, &pixmap);
      if ( nr < 1 )
        printf( "** Syntax error in file %s, line %d", filename, *line_cnt);

      if ( strcmp( type, "{") != 0)
        printf( "** Syntax error in file %s, line %d", filename, *line_cnt);

      path_cnt = 0;
      while( 1)
      {
        fp->getline( line, sizeof( line));
        if ( line[0] == 0)
          break;
        (*line_cnt)++;
        if ( line[0] == '!' || line[0] == '#')
          continue;
        nr = sscanf( line, "%s %s %s %d", type, name, file, &pixmap);
        if ( nr < 1 )
          printf( "** Syntax error in file %s, line %d", filename, *line_cnt);
        if ( strcmp( type, "}") == 0)
        {
          if ( nr != 1 )
            printf( "** Syntax error in file %s, line %d", filename, *line_cnt);
          break;
        }
        else
        {
          if ( path_cnt > 10)
            break;

          strcpy( path[path_cnt], type);
          path_cnt++;
        }
      }
    }
  }

  return return_menu;
}

void SubPalette::menu_tree_free()
{
  menu_tree_free_children( menu_tree);
}

void SubPalette::menu_tree_free_children( subpalette_sMenu *first_child)
{
  subpalette_sMenu *menu_p, *next;

  menu_p = next = first_child;  
  while( next)
  {
    next = menu_p->next;
    free( (char *) menu_p);
  }
}

void SubPaletteBrow::brow_setup()
{
  brow_sAttributes brow_attr;
  unsigned long mask;

  mask = 0;
  mask |= brow_eAttr_indentation;
  brow_attr.indentation = 0.5;
  mask |= brow_eAttr_annotation_space;
  brow_attr.annotation_space = 0.5;
  brow_SetAttributes( ctx, &brow_attr, mask); 
  brow_SetCtxUserData( ctx, subpalette);

  brow_EnableEvent( ctx, flow_eEvent_MB1Click, flow_eEventType_CallBack, 
	subpalette_brow_cb);
  brow_EnableEvent( ctx, flow_eEvent_MB1DoubleClick, flow_eEventType_CallBack, 
	subpalette_brow_cb);
  brow_EnableEvent( ctx, flow_eEvent_Key_PF4, flow_eEventType_CallBack, 
	subpalette_brow_cb);
  brow_EnableEvent( ctx, flow_eEvent_Key_Return, flow_eEventType_CallBack, 
	subpalette_brow_cb);
  brow_EnableEvent( ctx, flow_eEvent_Key_Right, flow_eEventType_CallBack, 
	subpalette_brow_cb);
  brow_EnableEvent( ctx, flow_eEvent_Key_Left, flow_eEventType_CallBack, 
	subpalette_brow_cb);
  brow_EnableEvent( ctx, flow_eEvent_SelectClear, flow_eEventType_CallBack, 
	subpalette_brow_cb);
  brow_EnableEvent( ctx, flow_eEvent_ObjectDeleted, flow_eEventType_CallBack, 
	subpalette_brow_cb);
  brow_EnableEvent( ctx, flow_eEvent_Key_Up, flow_eEventType_CallBack, 
	subpalette_brow_cb);
  brow_EnableEvent( ctx, flow_eEvent_Key_Down, flow_eEventType_CallBack, 
	subpalette_brow_cb);
  brow_EnableEvent( ctx, flow_eEvent_Key_PageUp, flow_eEventType_CallBack, 
	subpalette_brow_cb);
  brow_EnableEvent( ctx, flow_eEvent_Key_PageDown, flow_eEventType_CallBack, 
	subpalette_brow_cb);
  brow_EnableEvent( ctx, flow_eEvent_ScrollUp, flow_eEventType_CallBack, 
	subpalette_brow_cb);
  brow_EnableEvent( ctx, flow_eEvent_ScrollDown, flow_eEventType_CallBack, 
	subpalette_brow_cb);
  brow_EnableEvent( ctx, flow_eEvent_Key_PF3, flow_eEventType_CallBack, 
	subpalette_brow_cb);
  brow_EnableEvent( ctx, flow_eEvent_Key_Tab, flow_eEventType_CallBack, 
	subpalette_brow_cb);
  brow_EnableEvent( ctx, flow_eEvent_Map, flow_eEventType_CallBack, 
	subpalette_brow_cb);
  brow_EnableEvent( ctx, flow_eEvent_MB3Press, flow_eEventType_CallBack, 
	subpalette_brow_cb);
}

//
// Backcall routine called at creation of the brow widget
// Enable event, create nodeclasses and insert the root objects.
//
int SubPalette::init_brow_cb( FlowCtx *fctx, void *client_data)
{
  SubPalette *subpalette = (SubPalette *) client_data;
  BrowCtx *ctx = (BrowCtx *)fctx;

  subpalette->brow = new SubPaletteBrow( ctx, (void *)subpalette);

  subpalette->brow->brow_setup();
  subpalette->brow->create_nodeclasses();

  // Create the root item
  subpalette->object_attr();

  return 1;
}

ItemLocalSubGraphs::ItemLocalSubGraphs( SubPalette *subpalette, 
	char *item_name, char *item_filename,
	brow_tNode dest, flow_eDest dest_code) :
	Item(subpalette_eItemType_LocalSubGraphs)
{

  strcpy( name, item_name);
  strcpy( filename, item_filename);

  brow_CreateNode( subpalette->brow->ctx, item_name, 
		subpalette->brow->nc_object, 
		dest, dest_code, (void *) this, 1, &node);

  brow_SetAnnotPixmap( node, 0, subpalette->brow->pixmap_map);

  brow_SetAnnotation( node, 0, item_name, strlen(item_name));
}

ItemFile::ItemFile( SubPalette *subpalette, 
	char *item_name, char *item_filename, int item_pixmap,
	brow_tNode dest, flow_eDest dest_code) :
	Item(subpalette_eItemType_File), pixmap(item_pixmap)
{

  strcpy( name, item_name);
  strcpy( filename, item_filename);

  brow_CreateNode( subpalette->brow->ctx, item_name, 
		subpalette->brow->nc_sub, 
		dest, dest_code, (void *) this, 1, &node);

  brow_SetAnnotPixmap( node, 0, subpalette->brow->pixmap_leaf);
  if ( 0 <= (pixmap-1) && (pixmap-1) < SUBP_PIXMAPS_SIZE && 
       subpalette->brow->pixmaps[pixmap-1])
    brow_SetAnnotPixmap( node, 1, subpalette->brow->pixmaps[pixmap-1]);

  brow_SetAnnotation( node, 0, item_name, strlen(item_name));
}

int ItemLocalSubGraphs::open_children( SubPalette *subpalette, double x, double y)
{
  double	node_x, node_y;
  int		child_exist;

  brow_GetNodePosition( node, &node_x, &node_y);

  if ( brow_IsOpen( node))
  {
    // Close
    brow_SetNodraw( subpalette->brow->ctx);
    brow_CloseNode( subpalette->brow->ctx, node);
    if ( brow_IsOpen( node) & subpalette_mOpen_Attributes)
      brow_RemoveAnnotPixmap( node, 1);
    if ( brow_IsOpen( node) & subpalette_mOpen_Children)
      brow_SetAnnotPixmap( node, 0, subpalette->brow->pixmap_map);
    brow_ResetOpen( node, subpalette_mOpen_All);
    brow_ResetNodraw( subpalette->brow->ctx);
    brow_Redraw( subpalette->brow->ctx, node_y);
  }
  else
  {
    int			sts;
    char		found_file[120];
    char		fname[120];
    char		dev[80], dir[80], file[80], type[80];
    int			version;
    char		text[80];
    char		*s;
    int			idx;
    char		file_str[5][80];
    int		        nr;
    int                 i;
    vector<LocalFile> fvect;

    // Create some children
    brow_SetNodraw( subpalette->brow->ctx);

    child_exist = 0;

    nr = dcli_parse( filename, ",", "", (char *)file_str,
		sizeof( file_str) / sizeof( file_str[0]), sizeof( file_str[0]), 0);
    for ( i = 0; i < nr; i++) {
      dcli_translate_filename( fname, file_str[i]);
      sts = dcli_search_file( fname, found_file, DCLI_DIR_SEARCH_INIT);
      if ( ODD(sts)) {
	LocalFile f;
	strcpy( f.name, found_file);
	fvect.push_back( f);
      }
      while ( ODD(sts)) {
        sts = dcli_search_file( fname, found_file, DCLI_DIR_SEARCH_NEXT);
        if ( ODD(sts)) {
	  LocalFile f;
	  strcpy( f.name, found_file);
	  fvect.push_back( f);
	}
      }
      dcli_search_file( fname, found_file, DCLI_DIR_SEARCH_END);
    }

    subpalette_sort( fvect);

    for ( i = 0; i < (int) fvect.size(); i++) {
      dcli_parse_filename( fvect[i].name, dev, dir, file, type, &version);
      cdh_ToLower( text, file);
      text[0] = toupper( text[0]);
      
      // Skip next pages in animations
      if ( !((s = strstr( text, "__p")) && sscanf( s+3, "%d", &idx))) {
	new ItemFile( subpalette, text, fvect[i].name, 0, node, flow_eDest_IntoLast);
	child_exist = 1;
      }
    }

    if ( child_exist)
    {
      brow_SetOpen( node, subpalette_mOpen_Children);
      brow_SetAnnotPixmap( node, 0, subpalette->brow->pixmap_openmap);
    }
    brow_ResetNodraw( subpalette->brow->ctx);
    if ( child_exist)
      brow_Redraw( subpalette->brow->ctx, node_y);
  }
  return 1;
}

int ItemLocalSubGraphs::close( SubPalette *subpalette, double x, double y)
{
  double	node_x, node_y;

  if ( brow_IsOpen( node))
  {
    // Close
    brow_GetNodePosition( node, &node_x, &node_y);
    brow_SetNodraw( subpalette->brow->ctx);
    brow_CloseNode( subpalette->brow->ctx, node);
    brow_SetAnnotPixmap( node, 0, subpalette->brow->pixmap_map);
    brow_ResetOpen( node, subpalette_mOpen_All);
    brow_ResetNodraw( subpalette->brow->ctx);
    brow_Redraw( subpalette->brow->ctx, node_y);
  }
  return 1;
}

ItemMenu::ItemMenu( SubPalette *subpalette, const char *item_name, 
	brow_tNode dest, flow_eDest dest_code, subpalette_sMenu **item_child_list,
	int item_is_root) :
	Item( subpalette_eItemType_Menu), child_list(item_child_list), 
	is_root(item_is_root)
{
  type = subpalette_eItemType_Menu;
  strcpy( name, item_name);
  if ( !is_root)
  {
    brow_CreateNode( subpalette->brow->ctx, name, subpalette->brow->nc_object,
		dest, dest_code, (void *)this, 1, &node);

    // Set pixmap
    if ( *child_list)
      brow_SetAnnotPixmap( node, 0, subpalette->brow->pixmap_map);
    else
      brow_SetAnnotPixmap( node, 0, subpalette->brow->pixmap_leaf);
    // Set object name annotation
    brow_SetAnnotation( node, 0, name, strlen(name));
  }
}

int ItemMenu::open_children( SubPalette *subpalette, double x, double y)
{
  int		action_open;

  if ( !is_root)
  {
    if ( !brow_IsOpen( node))
      action_open = 1;
    else 
      action_open = 0;
  }
  if ( action_open || is_root)
  {
    // Display childlist
    double	node_x, node_y;
    Item 		*item;
    subpalette_sMenu	*menu;

    if ( !is_root)
      brow_GetNodePosition( node, &node_x, &node_y);
    else
      node_y = 0;
    brow_SetNodraw( subpalette->brow->ctx);
    menu = *child_list;
    while ( menu)
    {
      switch ( menu->item_type)
      {
        case subpalette_eItemType_Menu:
          item = (Item *) new ItemMenu( subpalette, menu->title, node, 
		flow_eDest_IntoLast, &menu->child_list,
		0);
          break;
        case subpalette_eItemType_File:
          item = (Item *) new ItemFile( subpalette, menu->title, menu->file,
		menu->pixmap, node, flow_eDest_IntoLast);
          break;
        case subpalette_eItemType_LocalSubGraphs:
          item = (Item *) new ItemLocalSubGraphs( subpalette, 
		menu->title, menu->file, node, flow_eDest_IntoLast);
          break;
    
        default:
          ;
      }
      menu = menu->next;
      if ( !is_root)
      {
        brow_SetOpen( node, subpalette_mOpen_Children);
        brow_SetAnnotPixmap( node, 0, subpalette->brow->pixmap_openmap);
      }
    }
    brow_ResetNodraw( subpalette->brow->ctx);
    brow_Redraw( subpalette->brow->ctx, node_y);
  }
  else
    close( subpalette, x, y);
  return 1;
}

int ItemMenu::close( SubPalette *subpalette, double x, double y)
{
  double	node_x, node_y;

  if ( brow_IsOpen( node))
  {
    // Close
    brow_GetNodePosition( node, &node_x, &node_y);
    brow_SetNodraw( subpalette->brow->ctx);
    brow_CloseNode( subpalette->brow->ctx, node);
    if ( brow_IsOpen( node) & subpalette_mOpen_Attributes)
      brow_RemoveAnnotPixmap( node, 1);
    if ( brow_IsOpen( node) & subpalette_mOpen_Children)
      brow_SetAnnotPixmap( node, 0, subpalette->brow->pixmap_map);
    brow_ResetOpen( node, subpalette_mOpen_All);
    brow_ResetNodraw( subpalette->brow->ctx);
    brow_Redraw( subpalette->brow->ctx, node_y);
  }
  return 1;
}

