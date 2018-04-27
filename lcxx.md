# LC++
#### using C++17's `std::variant` to write a
#### lambda calculus

---

# Lambda Calculus Background

----

## A way of describing computability

Note: similar to turing machines; any computation that one can do is expressible
in the (untyped version of the) lambda calculus.

----

## Kinds of Expressions - Var

```ocaml
type e =
  | ...
  | Var of string
  | ...
```

```
x
```

Note: var is short for "variable". Any identifier works.
Also, this is how I'll write these examples; OCaml on top, to describe the impl,
and lambda calculus on bottom. e stands for expression.

----

## Kinds of Expressions - Abs

```ocaml
type e =
  | ...
  | Abs of string * e
  | ...
```

```
λx.e₁
(* written as *)
/x.e₁
```

Note: Abs, short for "Abstraction", is a function definition.

----

## Kinds of Expressions - App

```ocaml
type e =
  | ...
  | App of e * e
  | ...
```

```
e₁ e₂
```

Note: App, short for "Application", is a function call.

----

## Kinds of Expressions - Altogether

```ocaml
type e =
  | Var of string
  | Abs of string * e
  | App of e * e
```

Note: this is the definition of the lambda calculus in OCaml. Simple, right?
When we bring it into C++, it will not be this simple :P

----

## Rules

```ocaml
let eval : expr -> expr =
  function
  | App (Abs (name, e1), e2) ->
      replace name ~in:e1 ~with:(eval e2)
  | x -> x
```

for example:

```
(/x.x x) y
(* becomes *)
y y
```

Note: You can see this is kind of simple; I don't define `replace` because
it would be a lot of code for very little benefit. You all know how functions
work :) PRESENTER: discuss α-equivalence

----

## Multi-parameter Functions?

We commonly might write

```ocaml
fun s z -> s z
```

in a language like OCaml; how would we do the same in the lambda calculus?
Currying, also known as, a function returning a function.

```
/s./z.s z
(* which, with precedence, looks like *)
/s.(/z.s z)
```

Note: you can see that lexical scope is in full effect here; if you have
closures in your favorite language, that's how the lambda calculus acts.

----

## Let Bindings?

What about local variables?

```ocaml
let id = /x.x in
id z
```

We deal with those by using more functions!

```
(/id. id z) /x.x
```

----

## Numbers?

MOAR FUNCTIONS

```
zero : /s./z.z
one : /s./z.s z
two : /s./z.s s z
```

seeing a pattern?

Note: we can think of a number N as applying a function N times.

----

## Any Questions?

---

# `std::variant` Background

Note: Now we get into the C++

----

## A Tagged Union

`std::variant` was designed to introduce a way to have sum types in C++.

```ocaml
type number =
  | Int of int
  | Float of float
```

becomes

```cpp
using Number = std::variant<int, float>;
```

Note: check if everyone knows what `using` does. Make sure this makes sense.

---

# "Pattern Matching"

How do we figure out what is in these variants?

----

## The Built-in Solution - `if` (ugly as heck)

We could simply use `if-else` blocks:

```cpp
void print_number(Number n) {
  if (std::holds_alternative<int>(n)) {
    std::cout << std::get<int>(n);
  } else if (std::holds_alternative<float>(n)) {
    std::cout << std::get<float>(n);
  } else {
    std::abort(); // it's impossible!
  }
}
```

Note: throws `std::bad_variant_access` if you try to `get` the wrong type.

----

## The Built-in Solution - `if` (less ugly as heck)

```cpp
void print_number(Number n) {
  if (int* i = std::get_if<int>(&n)) {
    std::cout << *i;
  } else if (float* f = std::get_if<float>(&n)) {
    std::cout << *f;
  } else {
    std::abort(); // it's (still) impossible!
  }
}
```

----

## The Built-in Solution - `std::visit`

These are kind of ugly; how can we do this with less explicit control flow?

```cpp
void print_number(Number n) {
  struct Visitor {
    void operator()(int i) { std::cout << i; }
    void operator()(float f) { std::cout << f; }
  };
  std::visit(Visitor(), n);
}
```

----

## The Nice Solution

I really dislike all of these solutions; they're ugly, or confusing.
In OCaml, we just write:

```ocaml
let print_number n =
  match n with
  | Int i -> print_int i
  | Float f -> print_float f
```

So I wrote a little helper to do the ugly stuff for me, and now I write:

```cpp
void print_number(Number n) {
  match(n)(
    [](int i) { std::cout << i; },
    [](float f) { std::cout << f; });
}
```

Note: show implementation.

---

# Recursive ADTs?

Note: As you might know, C++ doesn't allow recursive types without pointers,
due to lack of GC. Ask: does everyone know what an AST is?

----

## Pointers

Two kinds of smart pointers - `std::unique_ptr`, and `std::shared_ptr`.

Note: `unique_ptr` is for one owner; I used this for the parsed AST.
`shared_ptr` is for shared ownership; I used this for the evaluation AST.

----

## `Parse_ast`

```cpp
class Parse_ast {
public:
  class Variable;
  class Call;
  class Lambda;

  Parse_ast(Variable v) : underlying_(std::move(v)) {}
  Parse_ast(Call v) : underlying_(std::move(v)) {}
  Parse_ast(Lambda v) : underlying_(std::move(v)) {}

private:
  std::variant<Variable, Call, Lambda> underlying_;
};
```

Note: I'm not gonna go over this in a lot of detail - it's kind of boring.
I use this version of the lambda calculus AST

----

## `Parse_ast::Call`

```cpp
class Parse_ast::Call {
  std::unique_ptr<Parse_ast> callee_;
  std::unique_ptr<Parse_ast> argument_;

public:
  Parse_ast const& callee() const { return *callee_; }
  Parse_ast const& argument() const { return *argument_; }

  explicit Call(Parse_ast callee, Parse_ast argument)
      : callee_(make_unique<Parse_ast>(move(callee))),
        argument_(make_unique<Parse_ast>(move(argument))) {}
};
```

Note: I'm only going over `Parse_ast` and `Call` because they're all I need for
this; you can see I have a pointer to a `Parse_ast` for both the callee and the
argument. Therefore, I don't have any actual recursion - I can just use pointers
to the `Parse_ast`. This is obviously super simple; it's mostly just
boilerplate.

---

# Reducing with De Bruijn

Note: now onto the interesting bit.

----

## `Ast`

```cpp
class Ast {
public:
  class Variable;      Ast(Variable e);
  class Free_variable; Ast(Free_variable e);
  class Call;          Ast(Call e);
  class Lambda;        Ast(Lambda e);
private:
  using Underlying_type =
      std::variant<
        Variable, Free_variable, Call, Lambda> const;
  std::shared_ptr<Underlying_type> underlying_;
};
```

Note: this is the cool bit. Notice that `std::variant` _works_ without
defining `Variable`, `Free_variable`, `Call`, and `Lambda`.
We just have a pointer to it. We use this representation because we want to be
able to copy bits of the AST around during evaluation.

----

## `Ast::Variable` and `Ast::Free_variable`

```cpp
class Ast::Variable {
  int index_;
public:
  int index() const noexcept { return index_; }
  explicit Variable(int index) noexcept : index_(index) {}
};

class Ast::Free_variable {
  std::string name_;
public:
  std::string const& name() const noexcept { return name_; }
  explicit Free_variable(std::string name)
      : name_(std::move(name)) {}
};
```

Note: this is where De-Bruijn comes in. It makes it so much easier to replace
if you don't have names in multiple places. `Call` and `Lambda` are as you'd
expect.

---

# Evaluation

Now onto real code, because this doesn't really fit on slides :P

Note: just show code here.
