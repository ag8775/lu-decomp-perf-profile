/*
 * tutorial_5/start_app.c
 *
 * Copyright (c) 2001 Cradle Technologies
 */

#include <clasm/quadinfo.h>
#include <clasm/control.h>
#include <clasm/syslib.h>
#include <ssi/peregs.h>
#include <ssi/ssiconst.h>
#include <ssi/ssi.h>
#include "start_app.h"

int value _SD;
int load_table_address _SD;

void start_app_on_pe(int pe_number, char *name)
{
  int attributes;
  int pc_address;
  int *table_address;
  int *ptr;
  int *reg;
  int SL_Base;
  int PL_Base;  // Initialization is required only when the USE_MY_PL attribute is used
  Program *program;

  ptr = &load_table_address;
  *ptr = (unsigned int)_pe_getreg(PE_BASE_SSYS); // PE_BASE_SSYS  (R18)  The System Loader
  /*
   * provides the Load Table address in R18
   */
  ptr = &load_table_address;
  table_address = (int *)*ptr;
  program = (Program *) (*(unsigned int *)&table_address + PROGRAMENTRY_OFFSET);
  pc_address = program->programStart;
  value = pc_address;
  SL_Base = get_quad_base_addr( pe_number ); // Assumes SL Base = Quad Local Memory Base
  attributes = USE_MY_SL;
  launch_program(name, pe_number, table_address, attributes, SL_Base, PL_Base, _pe_getreg(PE_BASE_PDRAM) /*the PD Base Address register */, pc_address);
}

/*
 * stop_pe()
 */
void stop_pe(void)
{
  int temp;
  int *table_address;
  int attributes;
  int *pointer;

  temp = -1; // turn off the run bit in terminate_program
  pointer = &load_table_address; // address where load table address is saved
  table_address = (int *)(*pointer); // address of load table
  attributes = FREE_PL | FREE_PD;   // terminate attributes
  terminate_program(temp, table_address, attributes);
  exit(0);
}
