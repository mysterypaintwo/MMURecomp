// FUN_00402920 @ 0x402920

undefined4 FUN_00402920(undefined4 param_1)

{
  undefined4 uStack_48;
  uint uStack_44;
  undefined4 local_40;
  undefined4 local_3c;
  undefined4 local_38;
  undefined1 *local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00643550;
  local_10 = ExceptionList;
  uStack_44 = DAT_006c1e94 ^ (int)register0x00000010 - 4U;
  ExceptionList = &local_10;
  local_14 = (undefined1 *)&uStack_48;
  FUN_00428210();
  FUN_00404060();
  local_8 = 0;
  uStack_48 = 0x40296a;
  uStack_48 = FUN_004014a0();
  local_8._0_1_ = 1;
  local_40 = uStack_48;
  local_3c = uStack_48;
  FUN_00402b80(param_1);
  local_8 = (uint)local_8._1_3_ << 8;
  FUN_004939f0();
  ExceptionList = local_10;
  return local_38;
}

