// FUN_00564180 @ 0x564180

undefined4 * __thiscall
FUN_00564180(undefined4 *param_1,
            basic_string<char,std::char_traits<char>,std::allocator<char>_> param_2)

{
  float10 fVar1;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> abStack_c0 [28];
  undefined1 *local_a4;
  undefined4 local_a0;
  undefined4 local_9c;
  undefined4 local_98;
  undefined4 local_90;
  undefined4 local_8c;
  undefined4 local_88;
  undefined1 *local_84;
  undefined4 local_80;
  undefined4 local_7c;
  undefined4 local_78;
  uint uStack_74;
  undefined4 local_68;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  puStack_c = &LAB_0063b802;
  local_10 = ExceptionList;
  uStack_74 = DAT_006c1e94 ^ (int)register0x00000010 - 4U;
  ExceptionList = &local_10;
  local_8 = 0;
  local_98 = 0x5641c2;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,std::char_traits<char>,std::allocator<char>_> *)&local_90,
             (basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             &param_2);
  FUN_0043c1c0();
  local_8 = CONCAT31(local_8._1_3_,1);
  *param_1 = Bullet::vftable;
  *(undefined1 *)(param_1 + 0x54) = 0;
  local_78 = 0x5641f4;
  FUN_0043bc00();
  param_1[0x57] = 0;
  local_78 = 0x56420e;
  fVar1 = (float10)FUN_0043c800();
  local_68 = (undefined4)(longlong)ROUND(fVar1);
  param_1[0x58] = local_68;
  local_84 = &LAB_00428030;
  local_80 = 0;
  local_7c = 0;
  local_78 = 0;
  local_90 = 0;
  local_8c = 0;
  local_88 = 0;
  local_a4 = &LAB_0041f720;
  local_a0 = 0;
  local_9c = 0;
  local_98 = 0;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_c0,"Glued");
  FUN_00401370();
  local_8 = 0xffffffff;
  local_78 = 0x56430b;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(&param_2);
  ExceptionList = local_10;
  return param_1;
}

