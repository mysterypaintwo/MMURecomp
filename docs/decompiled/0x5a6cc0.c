// FUN_005a6cc0 @ 0x5a6cc0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_005a6cc0(int param_1)

{
  float10 fVar1;
  
  if (*(char *)(param_1 + 0x188) != '\0') {
    if (*(float *)(param_1 + 0x184) <= (float)_DAT_00697ce0) {
      *(undefined1 *)(param_1 + 0x188) = 0;
      *(undefined1 *)(param_1 + 0xd0) = 0;
      FUN_004023e0();
    }
    else {
      fVar1 = (float10)FUN_0043e510();
      *(float *)(param_1 + 0x184) = (float)((float10)*(float *)(param_1 + 0x184) - fVar1);
    }
  }
  FUN_0043cf60();
  return;
}

