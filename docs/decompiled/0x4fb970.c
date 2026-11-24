// FUN_004fb970 @ 0x4fb970

void __fastcall FUN_004fb970(int param_1)

{
  bool bVar1;
  undefined4 uVar2;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> *this;
  undefined1 *puVar3;
  undefined1 local_18 [16];
  int local_8;
  
  FUN_00623fc0(param_1 + 0x58);
  uVar2 = FUN_0043bc20(0x28,0x28);
  puVar3 = local_18;
  FUN_00401930(puVar3,uVar2);
  uVar2 = FUN_004014c0(puVar3,uVar2);
  FUN_004026e0(uVar2);
  for (local_8 = 0; local_8 < 0x12; local_8 = local_8 + 1) {
    this = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)FUN_00402700();
    bVar1 = std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::empty(this);
    if (!bVar1) {
      FUN_00623fc0(param_1 + 0x2d8 + local_8 * 0xb8);
      uVar2 = FUN_0043bc20(0x38,local_8 * 8 + 0x28);
      FUN_004026e0(uVar2);
    }
  }
  FUN_00441b70(0);
  return;
}

