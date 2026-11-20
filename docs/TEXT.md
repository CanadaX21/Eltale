# The game's text: its two encodings, as the game draws them and as this source builds them

The game draws text with two fonts, and each font has an encoding of its own. A string's bytes mean nothing until you
know which font draws it.

| Font | What it draws | Drawn by | Definition | Codec |
|---|---|---|---|---|
| the dialogue font | everything but the HUD's strings: the dialogue bank's NPC and sign strings, the NPCs' names, the pickup message, the opening speech, the ending's texts, the menus' text | `DrawString` and the message window's code | `text/encoding.json` | `Encoding` in `tools/text_codec.py` |
| the HUD font | the monsters', bosses' and spells' names, and the placeholder for a spell not yet learned | `DrawHUDText` (the battle's monster card, the spell cross, the spell banner) | `text/hud_font.json` | `HudEncoding` in `tools/text_codec.py` |

Each definition is data: what every byte does, and which character each cell of the font's sheet draws. The codec
holds only what a definition cannot: the source text's syntax and, for the dialogue font, the rule that places the row
selects. Every tool that reads or writes the game's text goes through the one codec.

## The dialogue font

**The sheet.** `gFontSheet`, read whole by `loadTextTextureAndPal` (0xE790 bytes, with the four 16-colour palettes,
`gTextPalettes`): 30 strips of 304 x 13 CI4 texels (`FontStrip`), one per glyph row. A cell is 10 texels wide
(`gFontMetrics[0]`), so a strip holds 30 cells. The build extracts the sheet as `assets/font/font_sheet.ci4.png`.

**The bytes:**

| Bytes | What they do |
|---|---|
| 0x00-0x1D | a glyph: that cell of the current row |
| 0x7F | a space (the pen moves, nothing is drawn) |
| 0x80-0x9D | a row select: strip N of the sheet becomes the current row |
| 0xA0-0xBF | a palette (`gTextPalettes[N * 16]` as the TLUT) |
| 0xC0-0xCF, 0xD0-0xDF | the font, the spacing |
| 0xE0 | a new line |
| 0xE1-0xE8 | a text speed |
| 0xF0 | wait for a button, then a new page |
| 0xF2 | the save prompt (the inns) |
| 0xFE | a menu |
| 0xFF | the end of the string |

**The row-select rule** (every original string follows it): a glyph is preceded by a row select when no glyph came
directly before it, or when the glyph before it is in another row. The encoder writes those selects itself; a select
the rule would not write is kept in the source text as `{row N}`.

**The source text** (the dialogue bank's `assets/dialogue_bank/*.dialogue.txt`, the NPC files' name pools): a
character of the glyph table is its glyph; U+3000 or a space is 0x7F; `\n`, `\f` and `\e` are 0xE0, 0xF0 and 0xF2;
`{pal N}`, `{font N}`, `{spacing N}`, `{speed N}`, `{menu}`, `{row N}` and `{code XX}` are the other codes; `{R,C}` is
the cell in row R, column C, written so where the table gives the cell no character, or not one alone. The build
encodes the text back into the original bytes (`tools/text_build.py`). Changing a string and keeping the change:
[ASSETS.md](ASSETS.md).

**The glyph table** (`glyphs` in `text/encoding.json`): the 900 cells, each with its character, a grade and where its
character came from. 832 are verified, 1 corrected, 1 uncertain, 1 a second drawing of another cell, and 65 empty.

**How the table was established.** Two earlier tools named characters for the cells, and this project used them as
leads:
- eltale-font-tool (Iconoclast's Eltale Monsters Translator, 2024; its source states the GPL-3.0-or-later) named 484
  cells in `font.c`'s table;
- QuestEdit (a Quest 64 editor; its author is unknown and no licence was found) named the 351 kanji the first left
  unknown.

Only those cell-to-character facts were taken: no code, file or other material of either tool. Each lead was then
checked against the cell's own bitmap in the original Japanese ROM's sheet, rendered for all 30 rows. Where a bitmap
and a lead disagreed, the strings that use the cell decided.
- **Corrected:** cell 12,15. The bitmap is 糸 beside 吉, and the strings that use it read 結界 and 結晶. The leads gave
  決, which is cell 12,14.
- **A second drawing:** cell 22,22 draws 妃, as cell 11,14 does (4 of 130 texels differ, in shading). One character
  encodes to one cell, so the source text writes this one `{22,22}`.
- **Uncertain:** cell 18,5. The bitmap is a narrow radical (忄 or 牜) beside 生, with the bottom stroke running under
  both. Next to 性 at cell 17,19, 29 of its 130 texels differ in coverage. The sheet orders it last among the kanji
  read せい, before 石 (せき). 性 drawn a second time and 牲 both fit, and a 10 x 13 bitmap does not settle which. No
  dialogue string uses the cell. The table gives it no character, the source text writes it `{18,5}`, and the lead
  (性) stays recorded.

Each cell's `source` and, where they differ, its `lead` stay in the file, so the provenance of every character can be
traced.

## The HUD font

**The sheet.** `gHudFontImage`, a `HudImage` in the cutscene bank (resident from boot): a CI4 image 144 texels wide
and 45 high (`HudImage`'s word at 0x0C is the image's height), 18 columns by 5 rows of 8 x 9 cells. Rows 0 to 2 hold
the katakana in gojūon order, with the two voicing marks in columns 15 and 16. Row 3 holds the small kana, ー, ・, a
cell that draws the two letters "Lv", and the digits 1 to 3. Row 4 holds the six hiragana of つかえません. The colours
come from `gHudFontPalettes`, chosen by `DrawHUDText`'s colour argument.

**The bytes** (`DrawHUDText`, `GetHUDTextWidth`):

| Bytes | What they do |
|---|---|
| 0x00-0x7F | a glyph: the cell in row (byte & 0x70) >> 4, column byte & 0x0F; the pen moves 8 texels |
| 0x80 | the voiced mark over the glyph before it (column 15 of that glyph's row); the pen moves 1 |
| 0x81 | the semi-voiced mark, the same way from column 16 |
| 0x82 | a half space: the pen moves 4 |
| 0xFF | the end of the string |
| 0x83-0xFE | unused: `DrawHUDText` would draw the cell of the byte with bit 0x80 cleared while `GetHUDTextWidth` counts 0 |

So ガ is カ followed by 0x80, and パ is ハ followed by 0x81. A mark after a glyph of row 3 or 4 would draw that row's
column 15 or 16 instead (the digit 3, or nothing): the encoder refuses one.

**The source text** of a HUD string: the table's characters (`Lv` for its cell; ASCII digits stand for the fullwidth
ones too), a voiced or semi-voiced kana for its glyph and the mark, a space for 0x82, `{R,C}` for a cell the table
gives no character, and `{code XX}` for any other byte.

**The table** (`text/hud_font.json`), read cell by cell from the original ROM's sheet, with no outside table or tool:
59 cells verified (the original's strings use them and decode as words with them), 7 read from the bitmap that no
string uses (ツ ヌ ノ ヤ ユ ヨ ゥ), the 6 voicing-mark cells, and 18 empty.

**The check.** `tools/test_hud_text.py`, run from the repository's root after `./configure`, takes every HUD string
where the build takes it: the generated records for the monsters' and spells' names, and `baserom.jp.z64` through
`gBossNames` and `gUnlearnedSpellName`. All 136 must decode with no unnamed cell and encode back to their bytes.
`--list` prints them.

**Changing a HUD string.** The build writes the HUD names as bytes, not as source text: u8 arrays in the generated
records (`assets/monsters_*/monster_data.records.inc.c`, `assets/spell_bank/`). The bosses' names are spans the build
reads from the ROM (`src/game/boss_anims.c`). `HudEncoding().encode('...')` gives the bytes for a new text, and
[ASSETS.md](ASSETS.md) shows how a changed record file is kept.

## The names, in the game's Japanese

Decoded from the original ROM with the HUD font's table (`tools/test_hud_text.py --list`).

**The bosses** (`gBossNames`, by boss; the boss files' segment names are this source's):

| Boss | Name | Boss file |
|---|---|---|
| 0 | ソルバリン | `boss_solvaring` |
| 1 | ゼルス | `boss_zelse` |
| 2 | ネプティー | `boss_nepty` |
| 3 | シルフ | `boss_shilf` |
| 4 | ファーゴ | `boss_fargo` |
| 5 | ギルティー | `boss_guilty` |
| 6 | ギース | `boss_beigis` |
| 7 | マモン | `boss_mammon` |

**The monsters** (each monster set's name table, by `monsterID`):

| Set | ID | Symbol | Name |
|---|---|---|---|
| 0 | 0 | `gMonSet0Mon00Name` | ウェアラビット |
| 0 | 1 | `gMonSet0Mon01Name` | ヘルハウンド |
| 0 | 2 | `gMonSet0Mon02Name` | マンイーター |
| 0 | 3 | `gMonSet0Mon03Name` | ビッグマウス |
| 0 | 4 | `gMonSet0Mon04Name` | アンブレラン |
| 0 | 5 | `gMonSet0Mon05Name` | パラソリアン |
| 0 | 6 | `gMonSet0Mon06Name` | オークジュニア |
| 0 | 7 | `gMonSet0Mon07Name` | グレムリン |
| 0 | 8 | `gMonSet0Mon08Name` | スケルトン |
| 0 | 9 | `gMonSet0Mon09Name` | ゴーストハウンド |
| 0 | 10 | `gMonSet0Mon10Name` | アンシリーコート |
| 0 | 11 | `gMonSet0Mon11Name` | ウルフゴート |
| 1 | 0 | `gMonSet1Mon00Name` | ゴブリン |
| 1 | 1 | `gMonSet1Mon01Name` | キングフロッグ |
| 1 | 2 | `gMonSet1Mon02Name` | アポピス |
| 1 | 3 | `gMonSet1Mon03Name` | マッドドール |
| 1 | 4 | `gMonSet1Mon04Name` | デスハガー |
| 1 | 5 | `gMonSet1Mon05Name` | コボルト |
| 1 | 6 | `gMonSet1Mon06Name` | ワンダルローズ |
| 1 | 7 | `gMonSet1Mon07Name` | フェイスハガー |
| 1 | 8 | `gMonSet1Mon08Name` | ナイトフロッグ |
| 1 | 9 | `gMonSet1Mon09Name` | ウッドドール |
| 1 | 10 | `gMonSet1Mon10Name` | ダークゴブリン |
| 1 | 11 | `gMonSet1Mon11Name` | ホットリップス |
| 1 | 12 | `gMonSet1Mon12Name` | インビジブルストーカー |
| 1 | 13 | `gMonSet1Mon13Name` | トレント |
| 1 | 14 | `gMonSet1Mon14Name` | コカトリス |
| 2 | 0 | `gMonSet2Mon00Name` | サウザンドアイ |
| 2 | 1 | `gMonSet2Mon01Name` | ミミック |
| 2 | 2 | `gMonSet2Mon02Name` | クローラー |
| 2 | 3 | `gMonSet2Mon03Name` | スコーピオン |
| 2 | 4 | `gMonSet2Mon04Name` | バグベア |
| 2 | 5 | `gMonSet2Mon05Name` | ワイバーン |
| 2 | 6 | `gMonSet2Mon06Name` | スカルバット |
| 2 | 7 | `gMonSet2Mon07Name` | クライシェル |
| 2 | 8 | `gMonSet2Mon08Name` | ブラッディジェル |
| 2 | 9 | `gMonSet2Mon09Name` | キャタピラー |
| 2 | 10 | `gMonSet2Mon10Name` | フィッシュマン |
| 3 | 0 | `gMonSet3Mon00Name` | サンドマン |
| 3 | 1 | `gMonSet3Mon01Name` | サバティナ |
| 3 | 2 | `gMonSet3Mon02Name` | ナイトメア |
| 3 | 3 | `gMonSet3Mon03Name` | ブルーマン |
| 3 | 4 | `gMonSet3Mon04Name` | ピンクマンボウ |
| 3 | 5 | `gMonSet3Mon05Name` | グルームウイング |
| 3 | 6 | `gMonSet3Mon06Name` | オーガ |
| 3 | 7 | `gMonSet3Mon07Name` | コンデンスサンド |
| 3 | 8 | `gMonSet3Mon08Name` | レッドワイバーン |
| 3 | 9 | `gMonSet3Mon09Name` | フレイムメイン |
| 3 | 10 | `gMonSet3Mon10Name` | マグママンボウ |
| 3 | 11 | `gMonSet3Mon11Name` | レッドローズナイト |
| 3 | 12 | `gMonSet3Mon12Name` | ホワイトローズナイト |
| 4 | 0 | `gMonSet4Mon00Name` | オーク |
| 4 | 1 | `gMonSet4Mon01Name` | ゴースト |
| 4 | 2 | `gMonSet4Mon02Name` | ウィル・オー・ウィプス |
| 4 | 3 | `gMonSet4Mon03Name` | スプライト |
| 4 | 4 | `gMonSet4Mon04Name` | ジャックランタン |
| 4 | 5 | `gMonSet4Mon05Name` | スパイダー |
| 4 | 6 | `gMonSet4Mon06Name` | バイバレラ |
| 4 | 7 | `gMonSet4Mon07Name` | テンプティー |
| 4 | 8 | `gMonSet4Mon08Name` | ピクシー |
| 4 | 9 | `gMonSet4Mon09Name` | グランガチ |
| 4 | 10 | `gMonSet4Mon10Name` | サンダージェル |
| 4 | 11 | `gMonSet4Mon11Name` | ジャイアントアント |
| 5 | 0 | `gMonSet5Mon00Name` | ジャッジメント |
| 5 | 1 | `gMonSet5Mon01Name` | ペイルライダー |
| 5 | 2 | `gMonSet5Mon02Name` | ニドヘグ |
| 5 | 3 | `gMonSet5Mon03Name` | スプリガン |

**The spells** (`gSpellNames`, by element and index). A spell with levels ends in the Lv cell and its level. Index 10
of earth reads ウィークネスス, with its ス written twice, as the original's bytes have it:

| Element | Index | Symbol | Name |
|---|---|---|---|
| `ELEMENT_FIRE` | 0 | `gSpellName0_00` | ファイアーボール Lv1 |
| `ELEMENT_FIRE` | 1 | `gSpellName0_01` | ファイアーボール Lv2 |
| `ELEMENT_FIRE` | 2 | `gSpellName0_02` | パワーロッド Lv1 |
| `ELEMENT_FIRE` | 3 | `gSpellName0_03` | ホーミングアロー Lv1 |
| `ELEMENT_FIRE` | 4 | `gSpellName0_04` | ホットスチーム Lv1 |
| `ELEMENT_FIRE` | 5 | `gSpellName0_05` | ファイアーボール Lv3 |
| `ELEMENT_FIRE` | 6 | `gSpellName0_06` | コンプレッション |
| `ELEMENT_FIRE` | 7 | `gSpellName0_07` | パワーロッド Lv2 |
| `ELEMENT_FIRE` | 8 | `gSpellName0_08` | ファイアーピラー |
| `ELEMENT_FIRE` | 9 | `gSpellName0_09` | ホーミングアロー Lv2 |
| `ELEMENT_FIRE` | 10 | `gSpellName0_10` | ファイアーボム |
| `ELEMENT_FIRE` | 11 | `gSpellName0_11` | バンパイアタッチ |
| `ELEMENT_FIRE` | 12 | `gSpellName0_12` | マグマボール |
| `ELEMENT_FIRE` | 13 | `gSpellName0_13` | エクスティンクション |
| `ELEMENT_FIRE` | 14 | `gSpellName0_14` | ホットスチーム Lv2 |
| `ELEMENT_EARTH` | 0 | `gSpellName1_00` | ロック Lv1 |
| `ELEMENT_EARTH` | 1 | `gSpellName1_01` | ロック Lv2 |
| `ELEMENT_EARTH` | 2 | `gSpellName1_02` | スピリットアーマー Lv1 |
| `ELEMENT_EARTH` | 3 | `gSpellName1_03` | ローリングロック Lv1 |
| `ELEMENT_EARTH` | 4 | `gSpellName1_04` | ウィークネス Lv1 |
| `ELEMENT_EARTH` | 5 | `gSpellName1_05` | ロック Lv3 |
| `ELEMENT_EARTH` | 6 | `gSpellName1_06` | マグネットロック |
| `ELEMENT_EARTH` | 7 | `gSpellName1_07` | スピリットアーマー Lv2 |
| `ELEMENT_EARTH` | 8 | `gSpellName1_08` | ロックコンボ |
| `ELEMENT_EARTH` | 9 | `gSpellName1_09` | コンフュージョンマジック |
| `ELEMENT_EARTH` | 10 | `gSpellName1_10` | ウィークネスス Lv2 |
| `ELEMENT_EARTH` | 11 | `gSpellName1_11` | ロックシャワー |
| `ELEMENT_EARTH` | 12 | `gSpellName1_12` | マジックバリア |
| `ELEMENT_EARTH` | 13 | `gSpellName1_13` | ローリングロック Lv2 |
| `ELEMENT_EARTH` | 14 | `gSpellName1_14` | ワイドウィークネス |
| `ELEMENT_WATER` | 0 | `gSpellName2_00` | ウォーターピラー Lv1 |
| `ELEMENT_WATER` | 1 | `gSpellName2_01` | ウォーターピラー Lv2 |
| `ELEMENT_WATER` | 2 | `gSpellName2_02` | ヒーリング Lv1 |
| `ELEMENT_WATER` | 3 | `gSpellName2_03` | ソウルサーチ Lv1 |
| `ELEMENT_WATER` | 4 | `gSpellName2_04` | ウォーターピラー Lv3 |
| `ELEMENT_WATER` | 5 | `gSpellName2_05` | アイスガス |
| `ELEMENT_WATER` | 6 | `gSpellName2_06` | アイスナイフ |
| `ELEMENT_WATER` | 7 | `gSpellName2_07` | イグジット |
| `ELEMENT_WATER` | 8 | `gSpellName2_08` | エスケープ |
| `ELEMENT_WATER` | 9 | `gSpellName2_09` | リターン |
| `ELEMENT_WATER` | 10 | `gSpellName2_10` | ヒーリング Lv2 |
| `ELEMENT_WATER` | 11 | `gSpellName2_11` | ソウルサーチ Lv2 |
| `ELEMENT_WATER` | 12 | `gSpellName2_12` | ウォーキングウォーター |
| `ELEMENT_WATER` | 13 | `gSpellName2_13` | ドレインマジック |
| `ELEMENT_WATER` | 14 | `gSpellName2_14` | インバリディティ |
| `ELEMENT_WIND` | 0 | `gSpellName3_00` | ウインドカッター Lv1 |
| `ELEMENT_WIND` | 1 | `gSpellName3_01` | ウインドカッター Lv2 |
| `ELEMENT_WIND` | 2 | `gSpellName3_02` | リストリクション Lv1 |
| `ELEMENT_WIND` | 3 | `gSpellName3_03` | アボイダンス Lv1 |
| `ELEMENT_WIND` | 4 | `gSpellName3_04` | サイレンス Lv1 |
| `ELEMENT_WIND` | 5 | `gSpellName3_05` | ウインドカッター Lv3 |
| `ELEMENT_WIND` | 6 | `gSpellName3_06` | ラージカッター |
| `ELEMENT_WIND` | 7 | `gSpellName3_07` | リストリクション Lv2 |
| `ELEMENT_WIND` | 8 | `gSpellName3_08` | ウインドボム |
| `ELEMENT_WIND` | 9 | `gSpellName3_09` | アボイダンス Lv2 |
| `ELEMENT_WIND` | 10 | `gSpellName3_10` | サイクロン |
| `ELEMENT_WIND` | 11 | `gSpellName3_11` | スローエネミー |
| `ELEMENT_WIND` | 12 | `gSpellName3_12` | ウインドウォーク |
| `ELEMENT_WIND` | 13 | `gSpellName3_13` | サイレンス Lv2 |
| `ELEMENT_WIND` | 14 | `gSpellName3_14` | アルティメットウィンド |

The placeholder for a spell not yet learned (`gUnlearnedSpellName`, drawn in its place on the spell cross) reads
つかえません.
