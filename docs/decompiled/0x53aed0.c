// FUN_0053aed0 @ 0x53aed0

void __fastcall FUN_0053aed0(int *param_1)

{
  bool bVar1;
  char cVar2;
  undefined4 *puVar3;
  int iVar4;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> *pbVar5;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> abStack_a0 [4];
  undefined4 uStack_9c;
  undefined4 uStack_98;
  undefined1 *puStack_94;
  int iStack_90;
  undefined1 *puStack_8c;
  char *pcStack_88;
  int iStack_84;
  undefined1 *puStack_80;
  undefined1 local_3c [8];
  undefined1 local_34 [8];
  undefined1 local_2c [8];
  undefined4 *local_24;
  undefined4 *local_20;
  int local_1c;
  undefined4 local_18;
  undefined1 local_14 [8];
  undefined4 local_c;
  undefined4 local_8;
  
  *(undefined1 *)(param_1 + 0x3c) = 1;
  puStack_80 = (undefined1 *)0x53aef3;
  bVar1 = std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::empty
                    ((basic_string<char,std::char_traits<char>,std::allocator<char>_> *)
                     (param_1 + 0x2e));
  if (bVar1) {
    *(undefined1 *)(param_1 + 0x3c) = 0;
    puStack_80 = (undefined1 *)0x1;
    iStack_84 = 1;
    pcStack_88 = (char *)0x53af17;
    pcStack_88 = std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::c_str
                           ((basic_string<char,std::char_traits<char>,std::allocator<char>_> *)
                            (param_1 + 0x35));
    puStack_8c = (undefined1 *)0x53af23;
    FUN_004026b0();
  }
  else {
    puStack_80 = (undefined1 *)0x1;
    iStack_84 = 0;
    pcStack_88 = (char *)0x53af38;
    pcStack_88 = std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::c_str
                           ((basic_string<char,std::char_traits<char>,std::allocator<char>_> *)
                            (param_1 + 0x2e));
    puStack_8c = (undefined1 *)0x53af44;
    FUN_004026b0();
  }
  puStack_80 = (undefined1 *)0x53af4c;
  FUN_00539660();
  puStack_80 = (undefined1 *)0x1;
  iStack_84 = 0x53af59;
  FUN_00562450();
  *(undefined1 *)(param_1 + 0x2b) = 1;
  *(undefined1 *)((int)param_1 + 0x9f) = 0;
  puStack_80 = (undefined1 *)0x53af75;
  FUN_0043bc00();
  puStack_80 = (undefined1 *)0x53af7a;
  FUN_00445770();
  puStack_80 = local_14;
  iStack_84 = param_1[1];
  pcStack_88 = (char *)0x53af92;
  (**(code **)(*param_1 + 0x20))();
  if (param_1[4] != 0) {
    local_24 = (undefined4 *)param_1[4];
    local_20 = local_24;
    if (local_24 != (undefined4 *)0x0) {
      pcStack_88 = (char *)0x1;
      puStack_8c = (undefined1 *)0x53afbe;
      (**(code **)*local_24)();
    }
    param_1[4] = 0;
  }
  *(undefined1 *)(param_1 + 0x28) = 0;
  pcStack_88 = local_2c;
  puStack_8c = (undefined1 *)0x53afed;
  puVar3 = (undefined4 *)FUN_00472660();
  local_c = *puVar3;
  local_8 = puVar3[1];
  while( true ) {
    pcStack_88 = local_34;
    puStack_8c = (undefined1 *)0x53b007;
    pcStack_88 = (char *)FUN_0046df60();
    puStack_8c = (undefined1 *)0x53b010;
    cVar2 = FUN_00426920();
    if (cVar2 == '\0') break;
    pcStack_88 = (char *)0x53b01f;
    puVar3 = (undefined4 *)FUN_004bdc80();
    local_18 = *puVar3;
    pcStack_88 = (char *)0x53b02c;
    local_1c = FUN_0042e9a0();
    pcStack_88 = (char *)0x1;
    puStack_8c = (undefined1 *)0x53b039;
    FUN_00621290();
    if (*(char *)(local_1c + 9) != '\0') {
      pcStack_88 = (char *)0x1;
      puStack_8c = (undefined1 *)0x53b04e;
      FUN_0042e9e0();
    }
    pcStack_88 = (char *)0x53b056;
    FUN_004bdc80();
    pcStack_88 = (char *)0x53b05d;
    iVar4 = FUN_0042e960();
    if (iVar4 == param_1[1]) {
      pcStack_88 = local_14;
      puStack_8c = (undefined1 *)0x53b071;
      FUN_004bdc80();
      puStack_8c = (undefined1 *)0x53b078;
      FUN_00621180();
    }
    pcStack_88 = (char *)0x0;
    puStack_8c = (undefined1 *)0x53b082;
    FUN_0042e9c0();
    pcStack_88 = (char *)0x0;
    puStack_8c = local_3c;
    iStack_90 = 0x53b090;
    FUN_0042f8a0();
  }
  pcStack_88 = (char *)0x53b09d;
  FUN_00539660();
  if ((undefined4 *)param_1[5] != (undefined4 *)0x0) {
    pcStack_88 = (char *)0x1;
    puStack_8c = (undefined1 *)0x53b0c0;
    (*(code *)**(undefined4 **)param_1[5])();
  }
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_a0,"Environment");
  iVar4 = FUN_00443e30();
  param_1[5] = iVar4;
  pcStack_88 = local_14;
  puStack_8c = (undefined1 *)0x53b102;
  FUN_00403ef0();
  pbVar5 = abStack_a0;
  iVar4 = param_1[1];
  (**(code **)(*param_1 + 0x18))(abStack_a0,iVar4);
  FUN_005b4970(pbVar5,iVar4);
  iStack_90 = param_1[1];
  puStack_94 = (undefined1 *)0x53b141;
  (**(code **)(*param_1 + 0x1c))();
  puStack_94 = (undefined1 *)0x53b148;
  FUN_00445000();
  if ((undefined4 *)param_1[3] != (undefined4 *)0x0) {
    puStack_94 = (undefined1 *)0x1;
    uStack_98 = 0x53b16b;
    (*(code *)**(undefined4 **)param_1[3])();
  }
  param_1[3] = 0;
  puStack_94 = (undefined1 *)0x0;
  uStack_98 = 0;
  uStack_9c = 0x53b190;
  FUN_004ac790();
  uStack_9c = 0x53b197;
  FUN_005025f0();
  puStack_94 = (undefined1 *)0x53b19e;
  FUN_00445040();
  puStack_94 = local_14;
  uStack_98 = 0x53b1ad;
  FUN_00403ef0();
  puStack_94 = (undefined1 *)0x53b1b5;
  FUN_0053a130();
  puStack_94 = (undefined1 *)0x53b1c0;
  iVar4 = FUN_0042e940();
  if (iVar4 == 1) {
    puStack_94 = (undefined1 *)0x53b1d0;
    FUN_00401930();
    puStack_94 = (undefined1 *)0x53b1dd;
    FUN_005b4850();
    puStack_94 = (undefined1 *)0x53b1f0;
    FUN_006253a0();
    puStack_94 = (undefined1 *)0x53b1fd;
    FUN_00401930();
    puStack_94 = (undefined1 *)0x53b204;
    FUN_006253a0();
    puStack_94 = (undefined1 *)0x53b20d;
    FUN_00445050();
  }
  else {
    puStack_94 = (undefined1 *)0x53b21a;
    FUN_00401930();
    puStack_94 = (undefined1 *)0x53b227;
    FUN_005b4870();
    puStack_94 = (undefined1 *)0x53b23b;
    FUN_006253a0();
    puStack_94 = (undefined1 *)0x53b248;
    FUN_00401930();
    puStack_94 = (undefined1 *)0x53b250;
    FUN_006253a0();
    puStack_94 = (undefined1 *)0x53b259;
    FUN_00445050();
  }
  return;
}

