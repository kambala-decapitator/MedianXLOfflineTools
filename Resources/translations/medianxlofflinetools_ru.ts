<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.0" language="ru_RU">
<context>
    <name>ClassName</name>
    <message>
        <location filename="../../enums.cpp" line="10"/>
        <source>Paladin</source>
        <translation>Паладин</translation>
    </message>
    <message>
        <location filename="../../enums.cpp" line="10"/>
        <source>Amazon</source>
        <translation>Амазонка</translation>
    </message>
    <message>
        <location filename="../../enums.cpp" line="10"/>
        <source>Sorceress</source>
        <translation>Колдунья</translation>
    </message>
    <message>
        <location filename="../../enums.cpp" line="10"/>
        <source>Necromancer</source>
        <translation>Некромант</translation>
    </message>
    <message>
        <location filename="../../enums.cpp" line="10"/>
        <source>Barbarian</source>
        <translation>Варвар</translation>
    </message>
    <message>
        <location filename="../../enums.cpp" line="10"/>
        <source>Druid</source>
        <translation>Друид</translation>
    </message>
    <message>
        <location filename="../../enums.cpp" line="10"/>
        <source>Assassin</source>
        <translation>Ассассин</translation>
    </message>
</context>
<context>
    <name>FindItemsDialog</name>
    <message>
        <location filename="../../finditemsdialog.ui" line="45"/>
        <source>Regular expression</source>
        <translation>Регулярное выражение</translation>
    </message>
    <message>
        <location filename="../../finditemsdialog.ui" line="58"/>
        <source>Case sensitive</source>
        <translation>Учитывать регистр</translation>
    </message>
    <message>
        <location filename="../../finditemsdialog.ui" line="74"/>
        <source>All greedy quantifiers become non-greedy, for example .+ and .* transform in .+? and .*? respectively</source>
        <translation>Все жадные квантификаторы становятся нежадными, например .+ и .* превращаются в .+? и .*? соответственно</translation>
    </message>
    <message>
        <location filename="../../finditemsdialog.ui" line="77"/>
        <source>Minimal matching</source>
        <translation>Минимальное совпадение</translation>
    </message>
    <message>
        <location filename="../../finditemsdialog.ui" line="90"/>
        <source>Wrap around</source>
        <translation>Зациклить поиск</translation>
    </message>
    <message>
        <location filename="../../finditemsdialog.ui" line="115"/>
        <source>Find previous</source>
        <translation>Найти предыдущий </translation>
    </message>
    <message>
        <location filename="../../finditemsdialog.ui" line="153"/>
        <source>Close</source>
        <translation>Закрыть</translation>
    </message>
    <message>
        <location filename="../../finditemsdialog.ui" line="175"/>
        <source>Find next</source>
        <translation>Найти следующий</translation>
    </message>
    <message>
        <location filename="../../finditemsdialog.ui" line="194"/>
        <source>Search in properties</source>
        <translation>Искать в свойствах</translation>
    </message>
    <message>
        <location filename="../../finditemsdialog.ui" line="213"/>
        <source>Treat string as single line (emulation of Perl&apos;s /s option)</source>
        <translation>Учитывать переносы строк (эмуляция Перловского ключа /s)</translation>
    </message>
    <message>
        <location filename="../../finditemsdialog.ui" line="216"/>
        <source>Multiline matching</source>
        <translation>Многострочное совпадение</translation>
    </message>
    <message>
        <location filename="../../finditemsdialog.ui" line="229"/>
        <source>Help</source>
        <translation>Справка</translation>
    </message>
    <message>
        <location filename="../../finditemsdialog.cpp" line="96"/>
        <location filename="../../finditemsdialog.cpp" line="131"/>
        <source>No more items found</source>
        <translation>Дальше предметов нет</translation>
    </message>
    <message>
        <location filename="../../finditemsdialog.cpp" line="159"/>
        <source>Hide results</source>
        <translation>Скрыть результаты</translation>
    </message>
    <message>
        <location filename="../../finditemsdialog.cpp" line="179"/>
        <source>Show results</source>
        <translation>Показать результаты</translation>
    </message>
    <message>
        <location filename="../../finditemsdialog.cpp" line="192"/>
        <source>Search help</source>
        <translation>Справка по поиску</translation>
    </message>
    <message>
        <location filename="../../finditemsdialog.cpp" line="198"/>
        <source>&lt;h3&gt;Item names&lt;/h3&gt;&lt;p&gt;All items except non-magical ones have their quality listed inside [] at the very beginning of item description. Valid values are: magic, rare, unique, set, crafted, honorific.&lt;/p&gt;&lt;p&gt;Runewords and charms are considered a special type of quality, so they have [runeword] and [charm] respectively.&lt;/p&gt;&lt;p&gt;Ethereal items also have [ethereal] in the end of the item name.&lt;/p&gt;&lt;p&gt;Set items have complete set name listed inside [] after set item name.&lt;/p&gt;&lt;p&gt;Personalized items have character name as it appears in game. The exception are items with affixes because affix display isn&apos;t supported in the current version of the application.&lt;/p&gt;&lt;p&gt;To see an example of such an item description, simply hover your mouse upon any item in the items window and look at the tooltip.&lt;/p&gt;&lt;h3&gt;Item properties&lt;/h3&gt;&lt;p&gt;If the &apos;Search in properties&apos; checkbox is checked, then the search is made not only by item name (as explained above), but also in item properties.&lt;/p&gt;&lt;p&gt;Properties appear the same way as they do in the item description view. Diablo color codes are also present here to simplify search for e.g. elite reanimates.&lt;/p&gt;&lt;h3&gt;Regular expressions&lt;/h3&gt;&lt;p&gt;Regular expressions syntax is mostly Perl-compatible, but there&apos;re some limitations. Refer to the Qt regular expressions description (http://developer.qt.nokia.com/doc/qregexp.html#details) for more information.&lt;/p&gt;&lt;p&gt;Regular expressions-only checkboxes in the dialog have tooltips on what they mean if it&apos;s not clear.&lt;/p&gt;&lt;p&gt;Hint: enter . (period) as a search text to see all your items :)&lt;/p&gt;&lt;h3&gt;Search results&lt;/h3&gt;&lt;p&gt;Hovering upon an item in the search results drop-down will display matched line with an actual match highlighted in &lt;b&gt;bold&lt;/b&gt;.&lt;/p&gt;&lt;p&gt;Double-clicking or pressing Return/Enter on an item shows it in the items window.&lt;/p&gt;</source>
        <translation>&lt;h3&gt;Названия предметов&lt;/h3&gt;&lt;p&gt;У всех предметов за исключением немагических название качества заключено в [] в самом начале их описания. Возможные значения: магический, редкий, уникальный, комплектный, крафтовый, почётный.&lt;/p&gt;&lt;p&gt;Рунные слова и талисманы имеют специальное качество, поэтому они имеют [рунное слово] и [талисман] соответственно.&lt;/p&gt;&lt;p&gt;Неземные (&quot;эфирные&quot;) предметы также имеют [неземной] в конце названия.&lt;/p&gt;&lt;p&gt;У комплектных предметов в [] после названия предмета указано название комплекта, которому они принадлежат.&lt;/p&gt;&lt;p&gt;У именовыных предметов имя персонажа появляется там же, где и в игре. Исключение составляют предметы с аффиксами, поскольку на данный момент поддержка аффиксов в приложении отсутствует.&lt;/p&gt;&lt;p&gt;Для того, чтобы увидеть подобное описание предмета, просто наведите курсор мыши на любой предмет в окне предметов и посмотрите на всплывающую подсказку.&lt;/p&gt;&lt;h3&gt;Свойства предметов&lt;/h3&gt;&lt;p&gt;Если отмечен чекбокс &apos;Искать в свойствах&apos;, тогда поиск ведётся не только по названию предмета (как описано выше), но также и в свойствах предмета.&lt;/p&gt;&lt;p&gt;Свойтсва записаны так же, как и в окне отображения свойств предмета. В них также присутствуют цветовые коды Диабло для упрощения поиска, скажем, элитных реанимэйтов.&lt;/p&gt;&lt;h3&gt;Регулярные выражения&lt;/h3&gt;&lt;p&gt;Синтаксис регулярных выражений во многом совместим с Перлом, но есть некоторые ограничения. Обратитесь к описанию регулярных выражений Qt (http://developer.qt.nokia.com/doc/qregexp.html#details) для более подробной информации.&lt;/p&gt;&lt;p&gt;Специальные чекбоксы только для регулярных выражений в диалоге имеют всплывающую подсказку с подробными объяснениями если непонятно что они делают.&lt;/p&gt;&lt;p&gt;Подсказка: введите . (точку) в качетве строки поиска для отображения всех ваших предметов :)&lt;/p&gt;&lt;h3&gt;Результаты поиска&lt;/h3&gt;&lt;p&gt;При наведении курсором на предмет в выпадающем окошке результатов поиска появится всплывающая подсказка с совпавшей строкой и реально совпавшими символами, выделенными &lt;b&gt;жирным&lt;/b&gt;.&lt;/p&gt;&lt;p&gt;Двойной шелчок или нажатие Return/Enter на предмете покажет его в окне предметов.&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../../finditemsdialog.cpp" line="340"/>
        <source>No items found</source>
        <translation>Предметов не найдено</translation>
    </message>
    <message>
        <location filename="../../finditemsdialog.cpp" line="380"/>
        <source>Find items</source>
        <translation>Поиск предметов</translation>
    </message>
</context>
<context>
    <name>FindResultsWidget</name>
    <message>
        <location filename="../../findresultswidget.cpp" line="34"/>
        <source>Base name</source>
        <translation>Основное имя</translation>
    </message>
    <message>
        <location filename="../../findresultswidget.cpp" line="34"/>
        <source>Special name</source>
        <translation>Специальное имя</translation>
    </message>
    <message>
        <location filename="../../findresultswidget.cpp" line="36"/>
        <source>Expand all</source>
        <translation>Развернуть все</translation>
    </message>
    <message>
        <location filename="../../findresultswidget.cpp" line="36"/>
        <source>Collapse all</source>
        <translation>Свернуть все</translation>
    </message>
    <message>
        <location filename="../../findresultswidget.cpp" line="102"/>
        <source>p. %1</source>
        <comment>page abbreviation</comment>
        <translatorcomment>аббревиатура страницы</translatorcomment>
        <translation>с. %1</translation>
    </message>
</context>
<context>
    <name>ItemDataBase</name>
    <message>
        <location filename="../../itemdatabase.cpp" line="29"/>
        <source>Reason: %1</source>
        <translation>Причина: %1</translation>
    </message>
    <message>
        <location filename="../../itemdatabase.cpp" line="35"/>
        <source>Error decrypting file &apos;%1&apos;</source>
        <translation>Ошибка при расшифровке файла &apos;%1&apos;</translation>
    </message>
    <message>
        <location filename="../../itemdatabase.cpp" line="71"/>
        <source>Items data not loaded.</source>
        <translation>Данные о предметах не загружены</translation>
    </message>
    <message>
        <location filename="../../itemdatabase.cpp" line="105"/>
        <source>Item types data not loaded.</source>
        <translation>Данные о типах предметов не загружены.</translation>
    </message>
    <message>
        <location filename="../../itemdatabase.cpp" line="127"/>
        <source>Properties data not loaded.</source>
        <translation>Данные о свойствах не загружены.</translation>
    </message>
    <message>
        <location filename="../../itemdatabase.cpp" line="169"/>
        <source>Sets data not loaded.</source>
        <translation>Данные о комплектах не загружены.</translation>
    </message>
    <message>
        <location filename="../../itemdatabase.cpp" line="198"/>
        <source>Skills data not loaded.</source>
        <translation>Данные о навыках не загружены.</translation>
    </message>
    <message>
        <location filename="../../itemdatabase.cpp" line="223"/>
        <source>Uniques data not loaded.</source>
        <translation>Данные об уникальных предметах не загружены.</translation>
    </message>
    <message>
        <location filename="../../itemdatabase.cpp" line="248"/>
        <source>Mystic Orbs data not loaded.</source>
        <translation>Данные о Мистических Сферах не загружены.</translation>
    </message>
    <message>
        <location filename="../../itemdatabase.cpp" line="274"/>
        <source>Monster names not loaded.</source>
        <translation>Имена монстров не загружены.</translation>
    </message>
    <message>
        <location filename="../../itemdatabase.cpp" line="296"/>
        <source>Runewords data not loaded.</source>
        <translation>Данные о рунных словах не загружены.</translation>
    </message>
    <message>
        <location filename="../../itemdatabase.cpp" line="323"/>
        <source>Socketables data not loaded.</source>
        <translation>Данные о предметах для гнёзд не загружены.</translation>
    </message>
    <message>
        <location filename="../../itemdatabase.cpp" line="367"/>
        <source>Non-magic qualities data not loaded.</source>
        <translation>Данные о немагических качествах не загружены.</translation>
    </message>
    <message>
        <location filename="../../itemdatabase.cpp" line="426"/>
        <source>%1&apos;s </source>
        <comment>personalized name</comment>
        <translatorcomment>имя в именованном предмете</translatorcomment>
        <translation>(%1) </translation>
    </message>
    <message>
        <location filename="../../itemdatabase.cpp" line="460"/>
        <source>magic</source>
        <translation>магический</translation>
    </message>
    <message>
        <location filename="../../itemdatabase.cpp" line="463"/>
        <source>set</source>
        <translation>комплектный</translation>
    </message>
    <message>
        <location filename="../../itemdatabase.cpp" line="466"/>
        <source>rare</source>
        <translation>редкий</translation>
    </message>
    <message>
        <location filename="../../itemdatabase.cpp" line="469"/>
        <source>charm</source>
        <translation>талисман</translation>
    </message>
    <message>
        <location filename="../../itemdatabase.cpp" line="469"/>
        <source>unique</source>
        <translation>уникальный</translation>
    </message>
    <message>
        <location filename="../../itemdatabase.cpp" line="472"/>
        <source>crafted</source>
        <translation>крафтовый</translation>
    </message>
    <message>
        <location filename="../../itemdatabase.cpp" line="475"/>
        <source>honorific</source>
        <translation>почётный</translation>
    </message>
    <message>
        <location filename="../../itemdatabase.cpp" line="483"/>
        <source>runeword</source>
        <translation>рунное слово</translation>
    </message>
    <message>
        <location filename="../../itemdatabase.cpp" line="495"/>
        <source>ethereal</source>
        <translation>небесный</translation>
    </message>
    <message>
        <location filename="../../itemdatabase.cpp" line="554"/>
        <source>Error loading &apos;%1&apos;</source>
        <translation>Ошибка при загрузке &apos;%1&apos;</translation>
    </message>
    <message>
        <location filename="../../itemdatabase.cpp" line="554"/>
        <source>Reason: %1</source>
        <comment>error with file</comment>
        <translatorcomment>ошибка при работе с файлом</translatorcomment>
        <translation>Причина: %1</translation>
    </message>
</context>
<context>
    <name>ItemParser</name>
    <message>
        <location filename="../../itemparser.cpp" line="20"/>
        <source>+%1% Enhanced Damage</source>
        <translation>+%1% Повышенный Урон</translation>
    </message>
    <message>
        <location filename="../../itemparser.cpp" line="129"/>
        <source>Item &apos;%1&apos; of unknown quality &apos;%2&apos; found!</source>
        <translation>Найден предмет &apos;%1&apos; неизвестного качества &apos;%2&apos;!</translation>
    </message>
    <message>
        <location filename="../../itemparser.cpp" line="178"/>
        <source>Error parsing item properties (ok == 0), please report!</source>
        <translation>Ошибка чтения свойств (ok == 0), пожалуйста сообщите!</translation>
    </message>
    <message>
        <location filename="../../itemparser.cpp" line="186"/>
        <source>Veterans</source>
        <translation>Ветераны</translation>
    </message>
    <message>
        <location filename="../../itemparser.cpp" line="186"/>
        <source>Trophy&apos;d</source>
        <translation>С трофеем</translation>
    </message>
    <message>
        <location filename="../../itemparser.cpp" line="187"/>
        <source>Blessed</source>
        <translation>Улучшен</translation>
    </message>
    <message>
        <location filename="../../itemparser.cpp" line="201"/>
        <source>Error parsing RW properties (ok == 0), please report!</source>
        <translation>Ошибка чтения свойств рунного слова (ok == 0), пожалуйста сообщите!</translation>
    </message>
    <message>
        <location filename="../../itemparser.cpp" line="235"/>
        <source>Unknown RW, please report!</source>
        <translation>Неизвестное рунное слово, пожалуйста сообщите!</translation>
    </message>
    <message>
        <location filename="../../itemparser.cpp" line="239"/>
        <source>Unknown RW (no socketables detected), please report!</source>
        <translation>Неизвестное рунное слово (отсутствуют предметы в гнёздах), пожалуйста сообщите!</translation>
    </message>
    <message>
        <location filename="../../itemparser.cpp" line="331"/>
        <source>+%1 to %2 Skill Levels</source>
        <comment>+x to class skills</comment>
        <translatorcomment>+х к навыкам класса</translatorcomment>
        <translation>+%1 к Навыкам (%2)</translation>
    </message>
    <message>
        <location filename="../../itemparser.cpp" line="335"/>
        <source>+%1 to %2</source>
        <comment>oskill</comment>
        <translatorcomment>оскилл</translatorcomment>
        <translation>+%1 к %2</translation>
    </message>
    <message>
        <location filename="../../itemparser.cpp" line="337"/>
        <source>(%1 Only)</source>
        <comment>class-specific skill</comment>
        <translatorcomment>классовый навык</translatorcomment>
        <translation>(Только %1)</translation>
    </message>
    <message>
        <location filename="../../itemparser.cpp" line="341"/>
        <source>Level %1 %2 (%3/%4 Charges)</source>
        <translation>Уровень %1 %2 (%3/%4 Зарядов)</translation>
    </message>
    <message>
        <location filename="../../itemparser.cpp" line="361"/>
        <source>Error parsing item properties (exception == %1), please report!</source>
        <translation>Ошибка чтения свойств (exception == %1), пожалуйста сообщите!</translation>
    </message>
</context>
<context>
    <name>ItemsPropertiesSplitter</name>
    <message>
        <location filename="../../itemspropertiessplitter.cpp" line="59"/>
        <source>Page #</source>
        <translation>Стр. №</translation>
    </message>
    <message>
        <location filename="../../itemspropertiessplitter.cpp" line="518"/>
        <source>Arcane Shards</source>
        <translation>Тайные Осколки</translation>
    </message>
    <message>
        <location filename="../../itemspropertiessplitter.cpp" line="525"/>
        <source>Signet of Learning</source>
        <translation>Печать Обучения</translation>
    </message>
    <message>
        <location filename="../../itemspropertiessplitter.cpp" line="295"/>
        <source>Disenchant into</source>
        <translation>Расщепить на</translation>
    </message>
    <message>
        <location filename="../../itemspropertiessplitter.cpp" line="538"/>
        <source>Remove Mystic Orbs</source>
        <translation>Удалить Мистические Сферы</translation>
    </message>
    <message>
        <location filename="../../itemspropertiessplitter.cpp" line="544"/>
        <source>Delete</source>
        <translation>Удалить</translation>
    </message>
    <message>
        <location filename="../../itemspropertiessplitter.cpp" line="454"/>
        <source>Are you sure you want to delete this item?</source>
        <translation>Вы точно хотите удалить этот предмет?</translation>
    </message>
    <message>
        <location filename="../../itemspropertiessplitter.cpp" line="458"/>
        <source>Cube is not empty. Do you really want to delete it?
Note: items inside will be preserved. You can recover them by getting new Cube.</source>
        <translation>Куб не пуст. Вы точно хотите его удалить?
Примечание: содержимое сохранится. Вы сможете восстановить его при получении нового Куба.</translation>
    </message>
</context>
<context>
    <name>ItemsViewerDialog</name>
    <message>
        <location filename="../../itemsviewerdialog.cpp" line="194"/>
        <source>Gear</source>
        <translation>Экипировка</translation>
    </message>
    <message>
        <location filename="../../itemsviewerdialog.cpp" line="194"/>
        <source>Inventory</source>
        <translation>Инвентарь</translation>
    </message>
    <message>
        <location filename="../../itemsviewerdialog.cpp" line="194"/>
        <source>Cube</source>
        <translation>Куб</translation>
    </message>
    <message>
        <location filename="../../itemsviewerdialog.cpp" line="194"/>
        <source>Stash</source>
        <translation>Сундук</translation>
    </message>
    <message>
        <location filename="../../itemsviewerdialog.cpp" line="194"/>
        <source>Personal Stash</source>
        <translation>Личный Сундук</translation>
    </message>
    <message>
        <location filename="../../itemsviewerdialog.cpp" line="194"/>
        <source>Shared Stash</source>
        <translation>Общий Сундук</translation>
    </message>
    <message>
        <location filename="../../itemsviewerdialog.cpp" line="194"/>
        <source>Hardcore Stash</source>
        <translation>Смертный Сундук</translation>
    </message>
    <message>
        <location filename="../../itemsviewerdialog.cpp" line="211"/>
        <source>Items viewer (items total: %1)</source>
        <translation>Просмотр предметов (всего предметов: %1)</translation>
    </message>
</context>
<context>
    <name>Language</name>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="729"/>
        <source>English</source>
        <comment>Your language name</comment>
        <translation>Русский</translation>
    </message>
</context>
<context>
    <name>MedianXLOfflineTools</name>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="51"/>
        <source>1337 hacker detected! Please, play legit.</source>
        <translation>Кулхацкер детектед! Пожалуйста, играйте честно.</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="53"/>
        <source>Max: %1</source>
        <translation>Максимум: %1</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="53"/>
        <source>Min: %1</source>
        <translation>Минимум: %1</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="53"/>
        <source>Invested: %1</source>
        <translation>Вложено: %1</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="92"/>
        <source>Language will be changed next time you run the application</source>
        <translation>Язык изменится при следующем запуске программы</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="104"/>
        <source>Load Character</source>
        <translation>Загрузить Персонажа</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="104"/>
        <source>Diablo 2 Save Files (*.d2s)</source>
        <translation>Файлы сохранений Diablo 2 (*.d2s)</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="106"/>
        <source>Character loaded</source>
        <translation>Персонаж загружен</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="118"/>
        <source>Do you really want to reload character? All unsaved changes will be lost.</source>
        <translation>Вы точно хотите перезагрузить персонажа? Все несохранённые изменения будут утеряны.</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="122"/>
        <source>Character reloaded</source>
        <translation>Персонаж перезагружен</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="374"/>
        <source>File &apos;%1&apos; successfully saved!</source>
        <translation>Файл &apos;%1&apos; успешно сохранён!</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="675"/>
        <source>About %1</source>
        <translation>О %1</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="679"/>
        <source>Released: %1</source>
        <translation>Выпущено: %1</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="682"/>
        <source>&lt;i&gt;Author:&lt;/i&gt; Filipenkov Andrey (&lt;b&gt;kambala&lt;/b&gt;)</source>
        <translation>&lt;i&gt;Автор:&lt;/i&gt; Филипенков Андрей (&lt;b&gt;kambala&lt;/b&gt;)</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="683"/>
        <source>&lt;a href=&quot;http://modsbylaz.14.forumer.com/viewtopic.php?t=23147&quot;&gt;Official Median XL Forum thread&lt;/a&gt;&lt;br&gt;&lt;a href=&quot;http://forum.worldofplayers.ru/showthread.php?t=34489&quot;&gt;Official Russian Median XL Forum thread&lt;/a&gt;</source>
        <translation>&lt;a href=&quot;http://modsbylaz.14.forumer.com/viewtopic.php?t=23147&quot;&gt;Тема на официальном форуме Median XL&lt;/a&gt;&lt;br&gt;&lt;a href=&quot;http://forum.worldofplayers.ru/showthread.php?t=34489&quot;&gt;Тема на официальном русском форуме Median XL&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="716"/>
        <source>&amp;Language</source>
        <comment>Language menu</comment>
        <translation>&amp;Язык</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="832"/>
        <location filename="../../medianxlofflinetools.cpp" line="868"/>
        <source>Name</source>
        <translation>Имя</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="836"/>
        <source>Class</source>
        <translation>Класс</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="838"/>
        <location filename="../../medianxlofflinetools.cpp" line="866"/>
        <source>Level</source>
        <translation>Уровень</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="840"/>
        <source>Title</source>
        <comment>Character title - Slayer/Champion/etc.</comment>
        <translation>Титул</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="864"/>
        <source>Type</source>
        <translation>Тип</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="875"/>
        <source>Inventory Gold</source>
        <translation>Золота в Инвентаре</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="878"/>
        <source>Stash Gold</source>
        <translation>Золота в Сундуке</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="888"/>
        <source>Strength</source>
        <translation>Сила</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="889"/>
        <source>Dexterity</source>
        <translation>Ловкость</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="890"/>
        <source>Vitality</source>
        <translation>Живучесть</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="891"/>
        <source>Energy</source>
        <translation>Энергиия</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="274"/>
        <location filename="../../medianxlofflinetools.cpp" line="380"/>
        <source>Error creating file &apos;%1&apos;</source>
        <translation>Ошибка при создании файла &apos;%1&apos;</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="355"/>
        <source>Error removing file &apos;%1&apos;</source>
        <translation>Ошибка при удалении файла &apos;%1&apos;</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="360"/>
        <source>Error renaming file &apos;%1&apos;</source>
        <translation>Ошибка при переименовании файла &apos;%1&apos;</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="377"/>
        <source>Error writing file &apos;%1&apos;</source>
        <translation>Ошибка при записи файла &apos;%1&apos;</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="636"/>
        <source>You have no free space in inventory and stash to store the Cube</source>
        <translation>У вас нет свободного места в инвентаре и сундуке для сохранения Куба</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="663"/>
        <source>Cube has been stored in %1 at (%2,%3)</source>
        <translation>Куб сохранён в %1 в клетку (%2,%3)</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="668"/>
        <source>Are you sure you want to disable automatic backups? Then don&apos;t blame me if your character gets corrupted.</source>
        <translation>Вы точно хотите отключить автоматическое создание резервных копий? Тогда не вините меня если вдруг ваш персонаж сломается.</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="685"/>
        <source>&lt;b&gt;Credits:&lt;/b&gt;&lt;ul&gt;&lt;li&gt;&lt;a href=&quot;http://modsbylaz.hugelaser.com/&quot;&gt;BrotherLaz&lt;/a&gt; for this awesome mod&lt;/li&gt;&lt;li&gt;&lt;a href=&quot;http://modsbylaz.14.forumer.com/profile.php?mode=viewprofile&amp;u=33805&quot;&gt;grig&lt;/a&gt; for the Perl source of &lt;a href=&quot;http://grig.vlexofree.com/&quot;&gt;Median XL Online Tools&lt;/a&gt; and tips&lt;/li&gt;&lt;li&gt;&lt;a href=&quot;http://phrozenkeep.hugelaser.com/index.php?ind=reviews&amp;op=section_view&amp;idev=4&quot;&gt;Phrozen Keep File Guides&lt;/a&gt; for tons of useful information on txt sources&lt;/li&gt;&lt;li&gt;&lt;a href=&quot;http://modsbylaz.14.forumer.com/profile.php?mode=viewprofile&amp;u=44046&quot;&gt;FixeR&lt;/a&gt; and &lt;a href=&quot;http://forum.worldofplayers.ru/member.php?u=84592&quot;&gt;Zelgadiss&lt;/a&gt; for intensive testing and tips on GUI &amp; functionality&lt;/li&gt;&lt;/ul&gt;</source>
        <translation>&lt;b&gt;Благодарности:&lt;/b&gt;&lt;ul&gt;&lt;li&gt;&lt;a href=&quot;http://modsbylaz.hugelaser.com/&quot;&gt;BrotherLaz&apos;у&lt;/a&gt; за этот шикарный мод&lt;/li&gt;&lt;li&gt;&lt;a href=&quot;http://modsbylaz.14.forumer.com/profile.php?mode=viewprofile&amp;u=33805&quot;&gt;grig&apos;у&lt;/a&gt; за Перловские исходники его &lt;a href=&quot;http://grig.vlexofree.com/&quot;&gt;Median XL Online Tools&lt;/a&gt; и за помощь&lt;/li&gt;&lt;li&gt;&lt;a href=&quot;http://phrozenkeep.hugelaser.com/index.php?ind=reviews&amp;op=section_view&amp;idev=4&quot;&gt;Phrozen Keep Гайдам&lt;/a&gt; за кучу полезной информации по текстовикам&lt;/li&gt;&lt;li&gt;&lt;a href=&quot;http://modsbylaz.14.forumer.com/profile.php?mode=viewprofile&amp;u=44046&quot;&gt;FixeR&apos;у&lt;/a&gt; и &lt;a href=&quot;http://forum.worldofplayers.ru/member.php?u=84592&quot;&gt;Zelgadiss&apos;у&lt;/a&gt; за интенсивное тестирование и советы по ГУИ и функциональности&lt;/li&gt;&lt;/ul&gt;</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="755"/>
        <source>Experience table data not loaded.</source>
        <translation>Таблица опыта не загружена.</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="769"/>
        <source>Mercenary names not loaded.</source>
        <translation>Имена наёмников не загружены.</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="881"/>
        <source>Free Skills</source>
        <translation>Невложеных Навыков</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="884"/>
        <source>Signets of Skill</source>
        <translation>Печатей Навыка</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="897"/>
        <source>Free Stats</source>
        <translation>Невложеных Атрибутов</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="900"/>
        <source>Signets of Learning</source>
        <translation>Печатей Обучения</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1108"/>
        <source>Error opening file &apos;%1&apos;</source>
        <translation>Ошибка при открытии файла &apos;%1&apos;</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1122"/>
        <source>Wrong file signature: should be 0x%1, got 0x%2.</source>
        <translation>Неверная сигнатура файла: должна быть 0x%1, а тут - 0x%2.</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1131"/>
        <source>Character checksum doesn&apos;t match. Looks like it&apos;s corrupted.</source>
        <translation>Контрольная сумма персонажа не совпадает. Похоже, он повреждён.</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1150"/>
        <source>This is not Expansion character.</source>
        <translation>Это не персонаж Расширения.</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1158"/>
        <source>Wrong class value: got %1</source>
        <translation>Некорректное значение класса: %1</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1165"/>
        <source>Wrong progression value: got %1</source>
        <translation>Некорректное значение прогресса: %1</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1172"/>
        <source>Wrong level: got %1</source>
        <translation>Некорректный уровень %1</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1199"/>
        <source>Wrong mercenary code: got %1</source>
        <translation>Некорректный код наёмника: %1</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1219"/>
        <source>Quests data not found!</source>
        <translation>Данные о квестах не найдены!</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1237"/>
        <source>Waypoint data not found!</source>
        <translation>Данные о телепортах не найдены!</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1264"/>
        <source>NPC data not found!</source>
        <translation>Данные о NPC не найдены!</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1271"/>
        <source>Stats data not found!</source>
        <translation>Данные о статистиках не найдены!</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1280"/>
        <source>Skills data not found!</source>
        <translation>Данные о навыках не найдены!</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1313"/>
        <source>Unknown statistic code found: %1. This is not Median XL character.</source>
        <translation>Найден неизвестный код статистики: %1. Этот персонаж - не из Median XL.</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1320"/>
        <source>Level in statistics (%1) isn&apos;t equal the one in header (%2).</source>
        <translation>Уровень в статистиках (%1) - не такой, как в заголовке (%2).</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1355"/>
        <source>Stats data is corrupted!</source>
        <translation>Данные о статистиках повреждены!</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1407"/>
        <source>Items data not found!</source>
        <translation>Данные о предметах не найдены!</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1430"/>
        <source>Items list doesn&apos;t have a terminator!</source>
        <translation>У списка вещей нету завершающего символа!</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1440"/>
        <source>Mercenary items section not found!</source>
        <translation>Секция с вещами наёмника не найдена!</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1448"/>
        <source>Save file is not terminated correctly!</source>
        <translation>Файл сохранения завершён некорректно!</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1455"/>
        <source>Mercenary items data is corrupted!</source>
        <translation>Данные о вещах наёмника повреждены!</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1475"/>
        <source>100% avoid is kewl</source>
        <translation>Ололо 100% авоид</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1477"/>
        <source>well, you have %1% actually</source>
        <comment>avoid</comment>
        <translation>ну, вообще-то у вас %1%</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1572"/>
        <source>Error opening PlugY stash &apos;%1&apos;</source>
        <translation>Ошибка при открытии сундука Плаги&apos;%1&apos;</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1590"/>
        <source>PlugY stash &apos;%1&apos; has wrong header</source>
        <translation>У сундука Плаги &apos;%1&apos; неправильный хедер</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1611"/>
        <source>Page %1 of &apos;%2&apos; has wrong PlugY header</source>
        <translation>У %1-й страницы &apos;%2&apos; неправильный хедер Плаги</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1617"/>
        <source>Page %1 of &apos;%2&apos; has wrong item header</source>
        <translation>У %1-й страницы &apos;%2&apos; неправильный хедер предмета</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1731"/>
        <source>Shared: %1</source>
        <comment>amount of gold in shared stash</comment>
        <translatorcomment>количество золота в общем сундуке</translatorcomment>
        <translation>Общий Сундук: %1</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1771"/>
        <source>DEAD</source>
        <comment>HC character is dead</comment>
        <translation>ТРУП</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1975"/>
        <source>Stats string is not byte aligned!</source>
        <translation>Строка статистик не выровнена по границе байта!</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="2026"/>
        <source>Error removing old backup &apos;%1&apos;</source>
        <translation>Ошибка при удалении старой резервной копии &apos;%1&apos;</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="2030"/>
        <source>Error creating backup of &apos;%1&apos;</source>
        <translation>Ошибка при создании резервной копии &apos;%1&apos;</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="2045"/>
        <source>Reason: %1</source>
        <comment>error with file</comment>
        <translation>Причина: %1</translation>
    </message>
</context>
<context>
    <name>MedianXLOfflineToolsClass</name>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="34"/>
        <source>Stats</source>
        <translation>Статистика</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="133"/>
        <source>Life</source>
        <translation>Жизнь</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="138"/>
        <source>Mana</source>
        <translation>Мана</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="143"/>
        <source>Stamina</source>
        <translation>Выносливость</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="148"/>
        <source>Current</source>
        <extracomment>life/mana/stamina</extracomment>
        <translation>Текущая</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="153"/>
        <source>Base</source>
        <extracomment>life/mana/stamina</extracomment>
        <translation>Базовая</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="219"/>
        <location filename="../../medianxlofflinetools.ui" line="809"/>
        <source>Respec Skills</source>
        <translation>Сбросить Навыки</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="232"/>
        <location filename="../../medianxlofflinetools.ui" line="823"/>
        <source>Activate Waypoints</source>
        <translation>Активировать Телепорты</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="309"/>
        <location filename="../../medianxlofflinetools.ui" line="795"/>
        <source>Respec Stats</source>
        <translation>Сбросить Атрибуты</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="329"/>
        <source>Character</source>
        <translation>Персонаж</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="341"/>
        <location filename="../../medianxlofflinetools.ui" line="834"/>
        <source>Rename</source>
        <translation>Переименовать</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="485"/>
        <location filename="../../medianxlofflinetools.ui" line="1037"/>
        <location filename="../../medianxlofflinetools.ui" line="1040"/>
        <source>Hardcore</source>
        <translation>Смертный</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="497"/>
        <location filename="../../medianxlofflinetools.ui" line="866"/>
        <source>Convert to Softcore</source>
        <translation>Сделать не Смертным</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="513"/>
        <location filename="../../medianxlofflinetools.ui" line="874"/>
        <source>Resurrect</source>
        <translation>Оживить</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="537"/>
        <source>Mercenary</source>
        <translation>Наёмник</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="594"/>
        <source>&amp;File</source>
        <translation>&amp;Файл</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="607"/>
        <source>&amp;Help</source>
        <translation>&amp;Справка</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="614"/>
        <source>&amp;Edit</source>
        <translation>&amp;Правка</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="627"/>
        <source>Options</source>
        <translation>Настройки</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="631"/>
        <source>Auto-open shared stashes</source>
        <translation>Авто-открытие общих сундуков</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="648"/>
        <source>Items</source>
        <translation>Предметы</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="655"/>
        <source>Go to page</source>
        <translation>Перейти на страницу</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="686"/>
        <location filename="../../medianxlofflinetools.ui" line="882"/>
        <source>Toolbar</source>
        <translation>Панель Инструментов</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="716"/>
        <source>L&amp;oad Character...</source>
        <translation>&amp;Загрузить Персонажа...</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="719"/>
        <source>Ctrl+O</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="734"/>
        <source>&amp;Save Character</source>
        <translation>&amp;Сохранить Персонажа</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="737"/>
        <source>Ctrl+S</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="745"/>
        <source>Exit</source>
        <translation>Выход</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="757"/>
        <source>About</source>
        <translation>О программе</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="798"/>
        <source>F5</source>
        <translation>F5</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="812"/>
        <source>F6</source>
        <translation>F6</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="826"/>
        <source>F7</source>
        <translation>F7</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="894"/>
        <source>Show items</source>
        <translation>Показать предметы</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="897"/>
        <source>Show Items</source>
        <translation>Показать Предметы</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="900"/>
        <source>Ctrl+I</source>
        <translation>Ctrl+I</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="914"/>
        <source>Load last used character</source>
        <translation>Загружать последнего персонажа</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="922"/>
        <source>Give me the Cube</source>
        <translation>Хочу Куб</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="925"/>
        <source>Present your character a new Horadric Cube</source>
        <translation>Подарите вашему любимчику новый Хорадрический Куб</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="933"/>
        <source>Auto-open items window</source>
        <translation>Авто-открытие окна предметов</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="936"/>
        <source>Open items window when a character is loaded</source>
        <translation>Открывать окно предметов сразу после загрузки персонажа</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="948"/>
        <source>Find...</source>
        <translation>Поиск...</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="951"/>
        <source>Find Items</source>
        <translation>Поиск Предметов</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="954"/>
        <source>Ctrl+F</source>
        <translation>Ctrl+F</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="965"/>
        <source>Find next</source>
        <translation>Найти следующий</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="976"/>
        <source>Find previous</source>
        <translation>Найти предыдущий </translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="990"/>
        <source>Ask before reloading character</source>
        <translation>Спросить при перезагрузке персонажа</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="993"/>
        <source>Show confirmation dialog when trying to reload a character</source>
        <translation>Спрашивать подтверждение при попытке перезагрузки персонажа</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="1001"/>
        <source>Always reload shared stashes</source>
        <translation>Всегда перезагружать общие сундуки</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="1004"/>
        <source>Always reload shared stashes when loading a character (may slow down the loading speed)</source>
        <translation>Всегда перезагружать общие сундуки при загрузке персонажа (может замедлить загрузку)</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="1015"/>
        <source>Personal</source>
        <translation>Личный</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="1026"/>
        <source>Shared</source>
        <translation>Общий</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="1051"/>
        <source>Make backups before saving</source>
        <translation>Создавать резервные копии перед сохранением</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="1062"/>
        <source>Warn when new name has color</source>
        <translation>Предупреждать если новое имя содержит цвет</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="1065"/>
        <source>Show confirmation dialog when using color in new name</source>
        <translation>Спрашивать подтверждение при попытке сохранения цветного имени</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="1074"/>
        <source>First</source>
        <translation>Первая</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="1089"/>
        <source>Previous 100</source>
        <translation>Назад на 100</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="1104"/>
        <source>Previous 10</source>
        <translation>Назад на 10</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="1119"/>
        <source>Previous</source>
        <translation>Назад</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="1134"/>
        <source>Next</source>
        <translation>Вперёд</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="1149"/>
        <source>Next 10</source>
        <translation>Вперёд на 10</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="1164"/>
        <source>Next 100</source>
        <translation>Вперёд на 100</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="1179"/>
        <source>Last</source>
        <translation>Последняя</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="760"/>
        <source>F1</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="775"/>
        <location filename="../../medianxlofflinetools.ui" line="778"/>
        <location filename="../../medianxlofflinetools.ui" line="781"/>
        <source>About Qt</source>
        <translation>О Qt</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="837"/>
        <source>Alt+N</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="849"/>
        <source>&amp;Reload Character</source>
        <translation>&amp;Перезагрузить Персонажа</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="852"/>
        <source>Ctrl+R</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>Mercenary</name>
    <message>
        <location filename="../../enums.cpp" line="32"/>
        <source>Ranger</source>
        <translation>Рейнджер</translation>
    </message>
    <message>
        <location filename="../../enums.cpp" line="32"/>
        <source>Priestess</source>
        <translation>Жрица</translation>
    </message>
    <message>
        <location filename="../../enums.cpp" line="32"/>
        <source>Exemplar</source>
        <translation>Храмовник</translation>
    </message>
    <message>
        <location filename="../../enums.cpp" line="32"/>
        <source>Shapeshifter</source>
        <translation>Оборотень</translation>
    </message>
    <message>
        <location filename="../../enums.cpp" line="32"/>
        <source>Fighter Mage</source>
        <translation>Боевой Маг</translation>
    </message>
    <message>
        <location filename="../../enums.cpp" line="33"/>
        <source>Necrolyte</source>
        <translation>Некролит</translation>
    </message>
    <message>
        <location filename="../../enums.cpp" line="33"/>
        <source>Bloodmage</source>
        <translation>Кровавый Маг</translation>
    </message>
    <message>
        <location filename="../../enums.cpp" line="33"/>
        <source>Abjurer</source>
        <translation>Отступник</translation>
    </message>
    <message>
        <location filename="../../enums.cpp" line="33"/>
        <source>Barbarian</source>
        <translation>Варвар</translation>
    </message>
</context>
<context>
    <name>Progression</name>
    <message>
        <location filename="../../enums.cpp" line="18"/>
        <source>Nobody</source>
        <translation>Никто</translation>
    </message>
    <message>
        <location filename="../../enums.cpp" line="20"/>
        <source>Destroyer</source>
        <translation>Разрушитель</translation>
    </message>
    <message>
        <location filename="../../enums.cpp" line="20"/>
        <source>Slayer</source>
        <translation>Убийца</translation>
    </message>
    <message>
        <location filename="../../enums.cpp" line="22"/>
        <source>Conqueror</source>
        <translation>Покоритель</translation>
    </message>
    <message>
        <location filename="../../enums.cpp" line="22"/>
        <source>Champion</source>
        <translation>Чемпион</translation>
    </message>
    <message>
        <location filename="../../enums.cpp" line="24"/>
        <source>Guardian</source>
        <translation>Страж</translation>
    </message>
    <message>
        <location filename="../../enums.cpp" line="24"/>
        <source>Patriarch</source>
        <translation>Патриарх</translation>
    </message>
    <message>
        <location filename="../../enums.cpp" line="24"/>
        <source>Matriarch</source>
        <translation>Матриарх</translation>
    </message>
</context>
<context>
    <name>PropertiesDisplayManager</name>
    <message>
        <location filename="../../propertiesdisplaymanager.cpp" line="34"/>
        <source>Item Level: %1</source>
        <translation>Уровень Предмета: %1</translation>
    </message>
    <message>
        <location filename="../../propertiesdisplaymanager.cpp" line="58"/>
        <source>Defense: %1</source>
        <translation>Защита: %1</translation>
    </message>
    <message>
        <location filename="../../propertiesdisplaymanager.cpp" line="66"/>
        <source>Durability</source>
        <translation>Прочность</translation>
    </message>
    <message>
        <location filename="../../propertiesdisplaymanager.cpp" line="70"/>
        <source>%1 of %2</source>
        <comment>durability</comment>
        <translatorcomment>прочность</translatorcomment>
        <translation>%1 из %2</translation>
    </message>
    <message>
        <location filename="../../propertiesdisplaymanager.cpp" line="75"/>
        <source>Quantity: %1</source>
        <translation>Количество: %1</translation>
    </message>
    <message>
        <location filename="../../propertiesdisplaymanager.cpp" line="77"/>
        <source>(%1 Only)</source>
        <comment>class-specific item</comment>
        <translatorcomment>классовый предмет</translatorcomment>
        <translation>(Только %1)</translation>
    </message>
    <message>
        <location filename="../../propertiesdisplaymanager.cpp" line="101"/>
        <source>Required Level: %1</source>
        <translation>Нужен Уровень: %1</translation>
    </message>
    <message>
        <location filename="../../propertiesdisplaymanager.cpp" line="126"/>
        <source>Armor</source>
        <translation>Броня</translation>
    </message>
    <message>
        <location filename="../../propertiesdisplaymanager.cpp" line="126"/>
        <source>Shield</source>
        <translation>Щит</translation>
    </message>
    <message>
        <location filename="../../propertiesdisplaymanager.cpp" line="126"/>
        <source>Weapon</source>
        <translation>Оружие</translation>
    </message>
    <message>
        <location filename="../../propertiesdisplaymanager.cpp" line="151"/>
        <source>+50% Damage to Undead</source>
        <translation>+50% Урон Нежити</translation>
    </message>
    <message>
        <location filename="../../propertiesdisplaymanager.cpp" line="153"/>
        <source>Socketed: (%1), Inserted: (%2)</source>
        <translation>Гнёзд: (%1), Вставлено: (%2)</translation>
    </message>
    <message>
        <location filename="../../propertiesdisplaymanager.cpp" line="155"/>
        <source>Ethereal (Cannot be Repaired)</source>
        <translation>Неземной (Нельзя Починить)</translation>
    </message>
    <message>
        <location filename="../../propertiesdisplaymanager.cpp" line="245"/>
        <source>Based on</source>
        <comment>&apos;based on level&apos; property; translate only if Median XL is translated into your language! (i.e. there&apos;s localized data in Resources/data/&lt;language&gt;)</comment>
        <translatorcomment>свойство &quot;основано на уровне&quot;; переводить только если Median XL переведён на ваш язык! (т.е. есть локализованные данные в Resources/data/&lt;язык&gt;)</translatorcomment>
        <translation>Основано на</translation>
    </message>
    <message>
        <location filename="../../propertiesdisplaymanager.cpp" line="288"/>
        <source>Repairs 1 Durability in %1 Seconds</source>
        <translation>Восстанавливает Единицу Прочности за %1 Секунд</translation>
    </message>
    <message>
        <location filename="../../propertiesdisplaymanager.cpp" line="305"/>
        <source>[special case %1, please report] %2 &apos;%3&apos; (id %4)</source>
        <translation>[особый случай %1, пожалуйста сообщите] %2 &apos;%3&apos; (id %4)</translation>
    </message>
    <message>
        <location filename="../../propertiesdisplaymanager.cpp" line="331"/>
        <source>They have Windows in Hell</source>
        <translation>У них есть Окна в Аду (Больше не существующая мини-игра)</translation>
    </message>
    <message>
        <location filename="../../propertiesdisplaymanager.cpp" line="334"/>
        <source>Mirror Mirror</source>
        <translation>Зеркало Зеркало (мини-игра 3)</translation>
    </message>
    <message>
        <location filename="../../propertiesdisplaymanager.cpp" line="337"/>
        <source>Countess</source>
        <translation>Графиня (мини-игра 1)</translation>
    </message>
    <message>
        <location filename="../../propertiesdisplaymanager.cpp" line="340"/>
        <source>Level Challenge 2</source>
        <translation>Уровневое Испытание 2</translation>
    </message>
    <message>
        <location filename="../../propertiesdisplaymanager.cpp" line="343"/>
        <source>Crowned</source>
        <translation>Коронация (мини-игра 2)</translation>
    </message>
    <message>
        <location filename="../../propertiesdisplaymanager.cpp" line="346"/>
        <source>Challenge with id %1 found, please report!</source>
        <translation>Найдено испытание с id %1, пожалуйста сообщите!</translation>
    </message>
</context>
<context>
    <name>PropertiesViewerWidget</name>
    <message>
        <location filename="../../propertiesviewerwidget.ui" line="30"/>
        <source>All</source>
        <translation>Все</translation>
    </message>
    <message>
        <location filename="../../propertiesviewerwidget.ui" line="47"/>
        <source>Item / MO</source>
        <translation>Вещь / МС</translation>
    </message>
    <message>
        <location filename="../../propertiesviewerwidget.ui" line="64"/>
        <source>RW / MO</source>
        <translation>РС / МС</translation>
    </message>
    <message>
        <location filename="../../propertiesviewerwidget.ui" line="81"/>
        <source>Socketables</source>
        <translation>В гнёздах</translation>
    </message>
    <message>
        <location filename="../../propertiesviewerwidget.cpp" line="88"/>
        <source>Item Level: %1</source>
        <translation>Уровень Предмета: %1</translation>
    </message>
    <message>
        <location filename="../../propertiesviewerwidget.cpp" line="112"/>
        <source>Defense: %1</source>
        <translation>Защита: %1</translation>
    </message>
    <message>
        <location filename="../../propertiesviewerwidget.cpp" line="121"/>
        <source>Durability</source>
        <translation>Прочность</translation>
    </message>
    <message>
        <location filename="../../propertiesviewerwidget.cpp" line="125"/>
        <source>%1 of %2</source>
        <comment>durability</comment>
        <translation>%1 из %2</translation>
    </message>
    <message>
        <location filename="../../propertiesviewerwidget.cpp" line="131"/>
        <source>Quantity: %1</source>
        <translation>Количество: %1</translation>
    </message>
    <message>
        <location filename="../../propertiesviewerwidget.cpp" line="134"/>
        <source>(%1 Only)</source>
        <comment>class-specific item</comment>
        <translation>(Только %1)</translation>
    </message>
    <message>
        <location filename="../../propertiesviewerwidget.cpp" line="167"/>
        <source>Required Level: %1</source>
        <translation>Нужен Уровень: %1</translation>
    </message>
    <message>
        <location filename="../../propertiesviewerwidget.cpp" line="183"/>
        <source>Armor</source>
        <translation>Броня</translation>
    </message>
    <message>
        <location filename="../../propertiesviewerwidget.cpp" line="183"/>
        <source>Shield</source>
        <translation>Щит</translation>
    </message>
    <message>
        <location filename="../../propertiesviewerwidget.cpp" line="183"/>
        <source>Weapon</source>
        <translation>Оружие</translation>
    </message>
    <message>
        <location filename="../../propertiesviewerwidget.cpp" line="197"/>
        <source>+50% Damage to Undead</source>
        <translation>+50% Урон Нежити</translation>
    </message>
    <message>
        <location filename="../../propertiesviewerwidget.cpp" line="199"/>
        <source>Socketed: (%1), Inserted: (%2)</source>
        <translation>Гнёзд: (%1), Вставлено: (%2)</translation>
    </message>
    <message>
        <location filename="../../propertiesviewerwidget.cpp" line="201"/>
        <source>Ethereal (Cannot be Repaired)</source>
        <translation>Неземной (Нельзя Починить)</translation>
    </message>
</context>
<context>
    <name>QD2CharRenamer</name>
    <message>
        <location filename="../../qd2charrenamer.cpp" line="25"/>
        <source>white</source>
        <translation>белый</translation>
    </message>
    <message>
        <location filename="../../qd2charrenamer.cpp" line="25"/>
        <source>red</source>
        <translation>красный</translation>
    </message>
    <message>
        <location filename="../../qd2charrenamer.cpp" line="25"/>
        <source>green</source>
        <translation>зелёный</translation>
    </message>
    <message>
        <location filename="../../qd2charrenamer.cpp" line="25"/>
        <source>blue</source>
        <translation>голубой</translation>
    </message>
    <message>
        <location filename="../../qd2charrenamer.cpp" line="25"/>
        <source>gold</source>
        <translation>золотой</translation>
    </message>
    <message>
        <location filename="../../qd2charrenamer.cpp" line="25"/>
        <source>dark gray</source>
        <translation>тёмно-серый</translation>
    </message>
    <message>
        <location filename="../../qd2charrenamer.cpp" line="25"/>
        <source>tan</source>
        <translation>жёлто-коричневый</translation>
    </message>
    <message>
        <location filename="../../qd2charrenamer.cpp" line="25"/>
        <source>orange</source>
        <translation>оранжевый</translation>
    </message>
    <message>
        <location filename="../../qd2charrenamer.cpp" line="25"/>
        <source>yellow</source>
        <translation>жёлтый</translation>
    </message>
    <message>
        <location filename="../../qd2charrenamer.cpp" line="25"/>
        <source>violet</source>
        <translation>фиолетовый</translation>
    </message>
    <message>
        <location filename="../../qd2charrenamer.cpp" line="81"/>
        <source>Character with colored name can&apos;t join multiplayer games. Are you sure you want to continue?</source>
        <translation>Персонаж с цветным именем не может участвовать в мультиплеерных играх. Вы точно хотите продолжить?</translation>
    </message>
    <message>
        <location filename="../../qd2charrenamer.cpp" line="93"/>
        <source>Rename (%1/15)</source>
        <comment>param is the number of characters in the name</comment>
        <translatorcomment>параметр - это количество символов в имени</translatorcomment>
        <translation>Переименовать (%1/15)</translation>
    </message>
    <message>
        <location filename="../../qd2charrenamer.cpp" line="133"/>
        <source>&quot;Dynamic&quot; colors below</source>
        <translation>Ниже идут &quot;динамические&quot; цвета</translation>
    </message>
</context>
<context>
    <name>QD2CharRenamerClass</name>
    <message>
        <location filename="../../qd2charrenamer.ui" line="41"/>
        <source>&lt;html&gt;&lt;body&gt;You can use the following ANSI characters (maximum 15 including colors):&lt;ul&gt;&lt;li&gt;codes 32-127: all except ?*&lt;&gt;.|:&quot;/\&lt;/li&gt;&lt;li&gt;codes 146, 147 and 160-191&lt;/li&gt;&lt;/ul&gt;Also name can&apos;t start with _ (underscore) or end with - (hyphen).&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;body&gt;
Вы можете использовать следующие ANSI символы (максимум 15 включая цвета):
&lt;ul&gt;
&lt;li&gt;коды 32-127: все кроме ?*&amp;lt;&amp;gt;.|:&amp;quot;/\&lt;/li&gt;
&lt;li&gt;коды 146, 147 и 160-191&lt;/li&gt;
&lt;/ul&gt;
Также имя не может начинаться с _ (подчёркивания) или заканчиваться на - (дефис).
&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../../qd2charrenamer.ui" line="51"/>
        <source>Color</source>
        <translation>Цвет</translation>
    </message>
</context>
<context>
    <name>ResurrectPenaltyDialog</name>
    <message>
        <location filename="../../resurrectpenaltydialog.ui" line="14"/>
        <source>Resurrect</source>
        <translation>Оживить</translation>
    </message>
    <message>
        <location filename="../../resurrectpenaltydialog.ui" line="20"/>
        <source>Choose your resurrection penalty</source>
        <translation>Выберите цену оживления</translation>
    </message>
    <message>
        <location filename="../../resurrectpenaltydialog.ui" line="26"/>
        <source>Pay with 10 levels</source>
        <translation>Потерять 10 уровней</translation>
    </message>
    <message>
        <location filename="../../resurrectpenaltydialog.ui" line="36"/>
        <source>Pay with 2.5% of skill points</source>
        <translation>Потерять 2.5% очков навыков</translation>
    </message>
    <message>
        <location filename="../../resurrectpenaltydialog.ui" line="43"/>
        <source>Pay with 4% of stat points</source>
        <translation>Потерять 4% очков атрибутов</translation>
    </message>
</context>
</TS>
