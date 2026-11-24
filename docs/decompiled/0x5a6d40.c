// FUN_005a6d40 @ 0x5a6d40

void __fastcall FUN_005a6d40(int *param_1)

{
  bool bVar1;
  char cVar2;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> *pbVar3;
  undefined4 *puVar4;
  undefined4 uVar5;
  int *piVar6;
  char *pcVar7;
  int local_30;
  undefined1 local_28 [8];
  undefined1 local_20 [8];
  int local_18;
  int *local_14;
  int local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  FUN_0043cc00();
  pcVar7 = "WilyBossExplosion";
  pbVar3 = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)FUN_00420900();
  bVar1 = std::operator==<char,std::char_traits<char>,std::allocator<char>_>(pbVar3,pcVar7);
  if (!bVar1) {
    pcVar7 = "MidBossExplosion";
    pbVar3 = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)FUN_00420900();
    bVar1 = std::operator==<char,std::char_traits<char>,std::allocator<char>_>(pbVar3,pcVar7);
    if ((!bVar1) && (cVar2 = (**(code **)(*param_1 + 0x58))(), cVar2 == '\0')) {
      _String_const_iterator<>();
      puVar4 = (undefined4 *)FUN_00472660(local_20);
      local_c = *puVar4;
      local_8 = puVar4[1];
      uVar5 = FUN_0046df60(local_28);
      cVar2 = FUN_00426920(uVar5);
      if (cVar2 != '\0') {
        piVar6 = (int *)FUN_004bdc80();
        local_10 = *piVar6;
        if (*(int *)(local_10 + 8) != -0x80000000) {
                    /* WARNING: Subroutine does not return */
          FUN_0042f830();
        }
        local_14 = param_1;
        cVar2 = FUN_0041f990();
        if (cVar2 == '\x01') {
          FUN_0043c800();
          local_30 = FUN_006253a0();
          local_30 = local_30 + 4;
        }
        else {
          FUN_0043c800();
          local_30 = FUN_006253a0();
        }
        local_18 = local_30;
        piVar6 = (int *)FUN_0043ed20();
        (**(code **)(*piVar6 + 0xc))(0,&local_18);
      }
    }
  }
  return;
}

