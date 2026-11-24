// FUN_00552100 @ 0x552100

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 * __fastcall FUN_00552100(undefined4 *param_1)

{
  float10 fVar1;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> abStack_e4 [28];
  undefined1 *local_c8;
  undefined4 local_c4;
  undefined4 local_c0;
  undefined4 local_bc;
  undefined4 local_b4;
  undefined4 local_b0;
  undefined4 local_ac;
  undefined1 *local_a8;
  undefined4 local_a4;
  undefined *local_a0;
  undefined4 *local_9c;
  uint uStack_98;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0064003e;
  local_10 = ExceptionList;
  uStack_98 = DAT_006c1e94 ^ (int)register0x00000010 - 4U;
  ExceptionList = &local_10;
  local_bc = 0x55213f;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,std::char_traits<char>,std::allocator<char>_> *)&local_b4,
             "Bullets\\WilyMachine-Shot\\WilyMachine-Shot.xgun");
  FUN_00564180();
  local_8 = 0;
  *param_1 = WilyMachineShot::vftable;
  local_9c = (undefined4 *)0x55216d;
  FUN_0043bc00();
  param_1[0x5c] = _DAT_00698384;
  param_1[0x5d] = 0;
  param_1[0x5e] = 0;
  param_1[0x5f] = 0;
  local_9c = (undefined4 *)0x5521ac;
  fVar1 = (float10)FUN_0043c800();
  param_1[0x5c] = (float)fVar1;
  local_a8 = &LAB_00425040;
  local_a4 = 0;
  local_a0 = (undefined *)0x0;
  local_9c = (undefined4 *)0x0;
  local_b4 = 0;
  local_b0 = 0;
  local_ac = 0;
  local_c8 = &LAB_00427fc0;
  local_c4 = 0;
  local_c0 = 0;
  local_bc = 0;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_e4,"Moving");
  FUN_00401370();
  local_a8 = &LAB_004242f0;
  local_a4 = 0;
  local_a0 = (undefined *)0x0;
  local_9c = (undefined4 *)0x0;
  local_b4 = 0;
  local_b0 = 0;
  local_ac = 0;
  local_c8 = &LAB_00426380;
  local_c4 = 0;
  local_c0 = 0;
  local_bc = 0;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_e4,"Wait");
  FUN_00401370();
  local_9c = param_1 + 1;
  local_a0 = &DAT_006764e8;
  local_a4 = 0x55235e;
  FUN_004405c0();
  ExceptionList = local_10;
  return param_1;
}

