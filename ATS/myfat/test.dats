
absviewtype absvt

viewtypedef
basket = $extype_struct "basket" of {
  empty = empty
, _rest = absvt
}

extern fun fill_basket (b: &basket? >> basket): int

extern fun clear_absvt (i: absvt >> absvt?): void

fun clear_basket (b: &basket >> basket?): void = let
  val () = clear_absvt (b._rest)
in
end



fun foo (): void = let
  var b: basket
  val ret = fill_basket (b)
  val () = clear_basket (b)
in
end


