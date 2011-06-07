// 
// absviewtype absvt
// 
// viewtypedef
// basket = $extype_struct "basket" of {
//   empty = empty
// , _rest = absvt
// }
// 
// extern fun fill_basket (b: &basket? >> basket): int
// 
// extern fun clear_absvt (i: absvt >> absvt?): void
// 
// fun clear_basket (b: &basket >> basket?): void = let
//   val () = clear_absvt (b._rest)
// in
// end
// 
// 
// 
// fun foo (): void = let
//   var b: basket
//   val ret = fill_basket (b)
//   val () = clear_basket (b)
// in
// end
// 
// 

fun level02 (x: &int): void = let
  val () = x := 3
in end

fun level01 (x: int): int = let
  // var y = x
  // val () = level02 (y)
  val () = level02 (x)
in
  x
end

implement main () = let
  val x = level01 (3)
in end

