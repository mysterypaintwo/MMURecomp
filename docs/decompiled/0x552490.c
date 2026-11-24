// FUN_00552490 @ 0x552490

undefined4 * FUN_00552490(void)

{
  basic_string<char,std::char_traits<char>,std::allocator<char>_> abStack_a8 [28];
  undefined1 *local_8c;
  undefined4 local_88;
  undefined4 local_84;
  undefined4 local_80;
  undefined4 local_78;
  undefined4 local_74;
  undefined4 local_70;
  undefined1 *local_6c;
  undefined4 local_68;
  undefined *local_64;
  undefined4 *local_60;
  uint uStack_5c;
  undefined4 local_58;
  undefined4 local_54;
  undefined4 local_50;
  undefined4 *local_4c;
  undefined1 *local_48;
  undefined1 *local_44;
  undefined4 local_40;
  undefined4 local_3c;
  undefined4 local_38;
  undefined1 *local_34;
  undefined4 local_30;
  undefined4 local_2c;
  undefined4 local_28;
  undefined1 *local_24;
  undefined4 local_20;
  undefined4 local_1c;
  undefined4 local_18;
  undefined1 *local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00640064;
  local_10 = ExceptionList;
  uStack_5c = DAT_006c1e94 ^ (int)register0x00000010 - 4U;
  ExceptionList = &local_10;
  local_14 = (undefined1 *)&local_78;
  local_80 = 0x5524cb;
  local_50 = std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
             basic_string<char,std::char_traits<char>,std::allocator<char>_>
                       ((basic_string<char,std::char_traits<char>,std::allocator<char>_> *)&local_78
                        ,"Bullets\\WilyMachine-Lightning\\WilyMachine-Lightning.xgun");
  local_54 = FUN_00564180();
  local_8 = 0;
  *local_4c = WilyMachineLightning::vftable;
  local_4c[0x59] = 0;
  *(undefined1 *)(local_4c + 0x5a) = 1;
  local_60 = (undefined4 *)0x55250b;
  FUN_0043bc00();
  local_24 = &LAB_004242f0;
  local_20 = 0;
  local_1c = 0;
  local_18 = 0;
  local_34 = &LAB_0041f710;
  local_30 = 0;
  local_2c = 0;
  local_28 = 0;
  local_44 = &LAB_00426380;
  local_40 = 0;
  local_3c = 0;
  local_38 = 0;
  local_6c = &LAB_004242f0;
  local_68 = 0;
  local_64 = (undefined *)0x0;
  local_60 = (undefined4 *)0x0;
  local_78 = 0;
  local_74 = 0;
  local_70 = 0;
  local_8c = &LAB_00426380;
  local_88 = 0;
  local_84 = 0;
  local_80 = 0;
  local_48 = abStack_a8;
  local_58 = std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
             basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_a8,"Idle");
  FUN_00401370();
  local_60 = local_4c + 1;
  local_64 = &DAT_00676568;
  local_68 = 0x5525e5;
  FUN_004405c0();
  ExceptionList = local_10;
  return local_4c;
}

