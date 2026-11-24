// FUN_0043c190 @ 0x43c190

void * __thiscall FUN_0043c190(void *param_1,byte param_2)

{
  FUN_0043c320();
  if ((param_2 & 1) != 0) {
    operator_delete(param_1);
  }
  return param_1;
}

