// FUN_005438b0 @ 0x5438b0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_005438b0(int param_1)

{
  float fVar1;
  float fVar2;
  bool bVar3;
  int iVar4;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> *pbVar5;
  float10 fVar6;
  char *pcVar7;
  
  iVar4 = FUN_00507ec0();
  if (iVar4 != 4) {
    fVar6 = (float10)FUN_0043e510();
    *(float *)(param_1 + 0x1ac) = (float)((float10)*(float *)(param_1 + 0x1ac) - fVar6);
    fVar1 = *(float *)(param_1 + 0x1ac);
    fVar2 = (float)_DAT_00697ce0;
    if ((NAN(fVar1) || NAN(fVar2)) || fVar1 < fVar2 == (fVar1 == fVar2)) {
      pcVar7 = "HurtKnockback";
      FUN_00462a90();
      pbVar5 = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)FUN_0043d890();
      bVar3 = std::operator==<char,std::char_traits<char>,std::allocator<char>_>(pbVar5,pcVar7);
      if (bVar3) {
        *(undefined1 *)(param_1 + 0xa5) = 1;
      }
    }
    else {
      *(undefined1 *)(param_1 + 0xa5) = 1;
    }
  }
  return;
}

