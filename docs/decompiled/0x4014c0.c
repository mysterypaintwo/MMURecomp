// FUN_004014c0 @ 0x4014c0

float * __thiscall FUN_004014c0(float *param_1,float *param_2,float *param_3)

{
  float fVar1;
  float fVar2;
  
  FUN_0043bc00();
  fVar1 = param_1[1];
  fVar2 = param_3[1];
  *param_2 = *param_1 + *param_3;
  param_2[1] = fVar1 + fVar2;
  return param_2;
}

