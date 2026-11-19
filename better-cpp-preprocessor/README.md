The **bits** directory is used to store the *rules* on converting **better c** syntax to **c** syntax.

**Rules** are applied to all of the occurrences, one rule at a time, top to bottom 
That allows combining them by applying a rule to the generated code by previous rules.
A rule can't match its own replacement recursively
since the replacements are stored in a buffer before applying.

**Rules** are arranged using the scheme below:

`{token}{token}...{token}={token}{token}...{token} ;`

Where the tokens on the **left** (or **left** tokens) represent **better c** syntax 
and on the **right** (or **right** tokens) represent **c** syntax to be replaced with.
(can be multiline)

**Note:** Any other text is treated as a *comment*.

Tokens belong to one of these groups:
- 'keyword'     -> Keywords (ex. string, int, ...)
- 'literal'     -> Identifiers (names) and Constants (ex. string literals)
- 'operator'    -> Operators (ex. +, -, =, ==, ...)
- NO GROUP NAME -> **USE ONLY THE NAMES** Special Symbols (ex. (, ), **\{**, **\}**, ;, :, ...)
- NO GROUP NAME -> literal text (ex. "Hello", could be used ex. to call internal functions) **USE WITH CAUTION**
- '...' token which is treated as an variable number of tokens

**Notes:** 

- Whitespaces are omitted when parsing rules,
  but they are used to divide special words (such as **OR**).

- Files in the **bits** directory are scanned in an arbitrary order.
  Please, place rules into one file if the order of applying is important.

- Even the last **rule** in a file should end with a `;`.

- If the parser reaches end of file, it will put all the **right** tokens
  of rules left on the internal stack and will print a warning if there are such.

- Braces **SHOULD** be matched.

- Braces match the first matching closing brace at the current nesting level.

- The `{...}` token should **NOT** be placed at the start nor at the end of a rule.

- The `{...}` token matches zero or more tokens. 
  It is greedy and matches as many tokens as possible
  while allowing the rest of the rule to match. It cannot cross a scope boundary.
  Scope boundaries are constituted by all balanced delimiters such as **()**, **[]**, **{}**.

- In the `{token}` You can use either the name of a group,
  a specific name of the token of a group,
  or a literal text (often necessary, **use with caution**).

- A token expression follows the syntax: `{group.type.index}`, 
  `{operator.+}` is treated the same as `{+}` and so on.
  `{literal.type}` matches only literals of given `type`.

- You can give an **index** to specific tokens (**highly encouraged**)
  by placing a **dot (.)** and an **index** (1-based) 
  to rearrange them.

- When indexes are omitted, tokens are automatically
  assigned indexes in the order they appear on the left side of
  the rule. These indexes determine the corresponding token order on the right side.

- Indexes refer to the position of a token within its group (rather than its absolute position in the rule).

- Among the tokens of the same group either **none** have their index specified or **all** of them.

- You can specify the type of a **literal** by putting the type name after the **dot (.)**.
  Types should come before indexes, following the token syntax given above.

- You can **negate** a token with `{NOT(token)}` notation.
  `{NOT(type)}` means matching all the types in the the same group as the `type`, except `type`.
  `{NOT(group)}` means matching all the groups, except `group`.
  `{NOT(group.type)}` is the same as `{NOT(type)}`.

- You can **combine** groups/types inside a single token with `{group/type OR group/type}` notation.
  (logical **or**)
  You can combine only tokens that are on the same position (have the same index)
  You can combine only groups with groups and types with types.

- You can allow for nested expressions  with `({token}{token} ...)`
  See **Examples** for clarification. TODO: give clarification. 
  kirkification.

- **Negation** can be used with **combination**.

- Source-code comments are omitted and not treated as tokens.

- 'Empty' token expressions (such as `{}`) are not valid tokens.

- Any malformed rules will result in a compilation error with the message informing about them.

**Examples:**

- ```
  {string} {literal.1} {=} {literal.2}
  =
  {string} {literal.1} {=} {"Example_constructor_function"} { ( } {literal.1} { , } {literal.2} { ) } ;
  ```
  The example above will turn

  `string S = "Hello, World!"`

  into

  `string S = Example_constructor_function(S, "Hello, World!")`

  (some spaces were omitted in this example to make it clearer,
  they will be placed before and after every token by the program)


- ```
  { \{ }
  {...}
  {literal.string.1} {=} {literal.2}
  {...}
  { \} }
  =
  { \{ }
  {...}
  {literal.string.1} {=} {literal.2}
  {...}
  {"free"} { ( } {literal.string.1} { ) }
  { \} }
  ;
  ```
  The example above will turn

  ```
  void f() {
    string s = "Hello";

    printf("%s\n", s);

    for(int i = 0; i < 2; ++i) {
        printf("%d ", i);
    }
  }
  ```
  into

  ```
  void f() {
    string s = "Hello";

    printf("%s\n", s);

    for(int i = 0; i < 2; ++i) {
        printf("%d ", i);
    }

    free(s);
  }
  ```