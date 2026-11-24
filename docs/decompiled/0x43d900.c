// FUN_0043d900 @ 0x43d900

void __thiscall FUN_0043d900(int param_1,float *param_2)

{
  float10 fVar1;
  
  if (*(char *)(param_1 + 0x68) == '\0') {
    fVar1 = (float10)FUN_00420130("TargetHotSpotX");
    fVar1 = -fVar1;
  }
  else {
    fVar1 = (float10)FUN_00420130("TargetHotSpotX");
  }
  *param_2 = (float)fVar1;
  fVar1 = (float10)FUN_00420130("TargetHotSpotY");
  param_2[1] = (float)fVar1;
  return;
}

