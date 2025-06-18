# Dialog String Format

Dialog String is a json array like `[1, "a", "b"]` but without `[` and `]`.

`"!char:<character_name>"` - Sets the character name for the dialog.

`"!place:<placement>"` - Sets the dialog placement (top, center, or bottom).
- `t` - Top
- `c` - Center
- `b` - Bottom

`"!"` - Makes the next line non-skippable.

`"<text>"` - Regular dialog text. [Read text format tags info...](https://wyliemaster.github.io/gddocs/#/topics/tags?id=tags)

## Example
```
"!char:John", "!place:c", "Hello, <d100><s260>how are you?</s>", "!", "This <d100>is <d050>a <d100><cr>non-skippable</c> <d030>line."
```