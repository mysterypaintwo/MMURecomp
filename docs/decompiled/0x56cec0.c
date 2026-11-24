// FUN_0056cec0 @ 0x56cec0

void __fastcall FUN_0056cec0(int param_1)

{
  undefined4 uVar1;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> abStack_44 [16];
  undefined4 uStack_34;
  undefined1 *puStack_30;
  int **ppiStack_2c;
  int *local_28;
  undefined4 local_24;
  int local_20;
  undefined1 local_1c [12];
  int *local_10;
  undefined1 *local_c;
  int *local_8;
  
  if (*(char *)(param_1 + 0x388) != '\0') {
    *(undefined1 *)(param_1 + 0x388) = 0;
    local_c = abStack_44;
    local_20 = param_1;
    local_24 = std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
               basic_string<char,std::char_traits<char>,std::allocator<char>_>
                         (abStack_44,"WilyMachinePillar");
    local_28 = (int *)FUN_00443e30();
    ppiStack_2c = &local_10;
    puStack_30 = local_1c;
    uStack_34 = 0x56cf25;
    local_10 = local_28;
    local_8 = local_28;
    FUN_00425310();
    ppiStack_2c = (int **)0x0;
    puStack_30 = (undefined1 *)0x56cf34;
    (**(code **)(*local_8 + 0x14))();
    puStack_30 = (undefined1 *)0x56cf3f;
    puStack_30 = (undefined1 *)FUN_00401930();
    uStack_34 = 0x56cf48;
    FUN_00403ef0();
    puStack_30 = (undefined1 *)0x56cf50;
    puStack_30 = (undefined1 *)FUN_0041efa0();
    uStack_34 = 0x56cf59;
    FUN_00425d70();
    puStack_30 = (undefined1 *)0x56cf61;
    uVar1 = FUN_0041efa0();
    *(undefined4 *)(local_20 + 0x36c) = uVar1;
  }
  return;
}

