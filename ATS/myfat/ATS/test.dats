absviewtype foo_ptr (l: addr) = $extype "int *"

viewtypedef basket = $extype_struct "basket_struct" of {
  empty = empty,
  foo = [l:agz] foo_ptr (l)
}

extern fun initbasket (b: &basket? >> basket): void
extern fun clearbasket (b: &basket >> basket?): void

extern fun use_foo {l:agz} (p: !foo_ptr l): void

fun test (): void = let
  var b: basket?
  val () = initbasket (b)
  val () = use_foo (b.foo)
  val () = clearbasket (b)
in
end


