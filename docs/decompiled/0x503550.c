// FUN_00503550 @ 0x503550

void __fastcall FUN_00503550(int param_1)

{
  char cVar1;
  bool bVar2;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> *pbVar3;
  int iVar4;
  undefined4 uVar5;
  undefined1 *puVar6;
  undefined1 local_14 [16];
  
  cVar1 = FUN_00443240();
  if ((cVar1 == '\0') && (*(char *)(param_1 + 0x17d0) != '\0')) {
    *(undefined1 *)(param_1 + 0x17d0) = 0;
    FUN_004026b0("GameOverLoop",1,1);
  }
  cVar1 = FUN_00441d90();
  if (cVar1 != '\0') {
    do {
      *(int *)(param_1 + 0x1794) = *(int *)(param_1 + 0x1794) + 1;
      FUN_004026b0("Select",0,0);
      if (9 < *(int *)(param_1 + 0x1794)) {
        *(undefined4 *)(param_1 + 0x1794) = 0;
      }
      pbVar3 = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)FUN_00402700();
      bVar2 = std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::empty(pbVar3);
    } while (bVar2);
  }
  cVar1 = FUN_00441d90();
  if (cVar1 != '\0') {
    do {
      *(int *)(param_1 + 0x1794) = *(int *)(param_1 + 0x1794) + -1;
      FUN_004026b0("Select",0,0);
      if (*(int *)(param_1 + 0x1794) < 0) {
        *(undefined4 *)(param_1 + 0x1794) = 9;
      }
      pbVar3 = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)FUN_00402700();
      bVar2 = std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::empty(pbVar3);
    } while (bVar2);
  }
  cVar1 = FUN_00441d90();
  if (cVar1 != '\0') {
    iVar4 = FUN_00402b30(*(undefined4 *)(param_1 + 0x1794));
    *(undefined4 *)(param_1 + 0x1798) = *(undefined4 *)(iVar4 + 0x1c);
    if ((*(int *)(param_1 + 0x1798) == 0x10) || (*(int *)(param_1 + 0x1798) == 1)) {
      FUN_004405c0("Confirm",param_1 + 4);
      return;
    }
    if (*(int *)(param_1 + 0x1798) != 0) {
      FUN_004026b0("Confirm",0,0);
      FUN_004405c0("Flash",param_1 + 4);
    }
  }
  uVar5 = FUN_0043bc20(0x28,*(int *)(param_1 + 0x1794) * 0x10 + 0x28);
  puVar6 = local_14;
  FUN_00401930(puVar6,uVar5);
  uVar5 = FUN_004014c0(puVar6,uVar5);
  FUN_004026e0(uVar5);
  return;
}

