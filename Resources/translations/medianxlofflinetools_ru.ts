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
    <name>ExperienceIndicatorGroupBox</name>
    <message>
        <location filename="../../experienceindicatorgroupbox.cpp" line="7"/>
        <source>Experience</source>
        <translation>Опыт</translation>
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
        <location filename="../../finditemsdialog.cpp" line="97"/>
        <location filename="../../finditemsdialog.cpp" line="132"/>
        <source>No more items found</source>
        <translation>Дальше предметов нет</translation>
    </message>
    <message>
        <location filename="../../finditemsdialog.cpp" line="160"/>
        <source>Hide results</source>
        <translation>Скрыть результаты</translation>
    </message>
    <message>
        <location filename="../../finditemsdialog.cpp" line="180"/>
        <source>Show results</source>
        <translation>Показать результаты</translation>
    </message>
    <message>
        <location filename="../../finditemsdialog.cpp" line="193"/>
        <source>Search help</source>
        <translation>Справка по поиску</translation>
    </message>
    <message>
        <location filename="../../finditemsdialog.cpp" line="199"/>
        <source>&lt;h3&gt;Item names&lt;/h3&gt;&lt;p&gt;All items except non-magical ones have their quality listed inside [] at the very beginning of item description. Valid values are: magic, rare, unique, set, crafted, honorific.&lt;/p&gt;&lt;p&gt;Runewords and charms are considered a special type of quality, so they have [runeword] and [charm] respectively.&lt;/p&gt;&lt;p&gt;Ethereal items also have [ethereal] in the end of the item name.&lt;/p&gt;&lt;p&gt;Set items have complete set name listed inside [] after set item name.&lt;/p&gt;&lt;p&gt;Personalized items have character name as it appears in game. The exception are items with affixes because affix display isn&apos;t supported in the current version of the application.&lt;/p&gt;&lt;p&gt;To see an example of such an item description, simply hover your mouse upon any item in the items window and look at the tooltip.&lt;/p&gt;&lt;h3&gt;Item properties&lt;/h3&gt;&lt;p&gt;If the &apos;Search in properties&apos; checkbox is checked, then the search is made not only by item name (as explained above), but also in item properties.&lt;/p&gt;&lt;p&gt;Properties appear the same way as they do in the item description view. Diablo color codes are also present here to simplify search for e.g. elite reanimates.&lt;/p&gt;&lt;h3&gt;Regular expressions&lt;/h3&gt;&lt;p&gt;Regular expressions syntax is mostly Perl-compatible, but there&apos;re some limitations. Refer to the Qt regular expressions description (http://developer.qt.nokia.com/doc/qregexp.html#details) for more information.&lt;/p&gt;&lt;p&gt;Regular expressions-only checkboxes in the dialog have tooltips on what they mean if it&apos;s not clear.&lt;/p&gt;&lt;p&gt;Hint: enter . (period) as a search text to see all your items :)&lt;/p&gt;&lt;h3&gt;Search results&lt;/h3&gt;&lt;p&gt;Hovering upon an item in the search results drop-down will display matched line with an actual match highlighted in &lt;b&gt;bold&lt;/b&gt;.&lt;/p&gt;&lt;p&gt;Double-clicking or pressing Return/Enter on an item shows it in the items window.&lt;/p&gt;</source>
        <translation>&lt;h3&gt;Названия предметов&lt;/h3&gt;&lt;p&gt;У всех предметов за исключением немагических название качества заключено в [] в самом начале их описания. Возможные значения: магический, редкий, уникальный, комплектный, крафтовый, почётный.&lt;/p&gt;&lt;p&gt;Рунные слова и талисманы имеют специальное качество, поэтому они имеют [рунное слово] и [талисман] соответственно.&lt;/p&gt;&lt;p&gt;Неземные (&quot;эфирные&quot;) предметы также имеют [неземной] в конце названия.&lt;/p&gt;&lt;p&gt;У комплектных предметов в [] после названия предмета указано название комплекта, которому они принадлежат.&lt;/p&gt;&lt;p&gt;У именовыных предметов имя персонажа появляется там же, где и в игре. Исключение составляют предметы с аффиксами, поскольку на данный момент поддержка аффиксов в приложении отсутствует.&lt;/p&gt;&lt;p&gt;Для того, чтобы увидеть подобное описание предмета, просто наведите курсор мыши на любой предмет в окне предметов и посмотрите на всплывающую подсказку.&lt;/p&gt;&lt;h3&gt;Свойства предметов&lt;/h3&gt;&lt;p&gt;Если отмечен чекбокс &apos;Искать в свойствах&apos;, тогда поиск ведётся не только по названию предмета (как описано выше), но также и в свойствах предмета.&lt;/p&gt;&lt;p&gt;Свойтсва записаны так же, как и в окне отображения свойств предмета. В них также присутствуют цветовые коды Диабло для упрощения поиска, скажем, элитных реанимэйтов.&lt;/p&gt;&lt;h3&gt;Регулярные выражения&lt;/h3&gt;&lt;p&gt;Синтаксис регулярных выражений во многом совместим с Перлом, но есть некоторые ограничения. Обратитесь к описанию регулярных выражений Qt (http://developer.qt.nokia.com/doc/qregexp.html#details) для более подробной информации.&lt;/p&gt;&lt;p&gt;Специальные чекбоксы только для регулярных выражений в диалоге имеют всплывающую подсказку с подробными объяснениями если непонятно что они делают.&lt;/p&gt;&lt;p&gt;Подсказка: введите . (точку) в качетве строки поиска для отображения всех ваших предметов :)&lt;/p&gt;&lt;h3&gt;Результаты поиска&lt;/h3&gt;&lt;p&gt;При наведении курсором на предмет в выпадающем окошке результатов поиска появится всплывающая подсказка с совпавшей строкой и реально совпавшими символами, выделенными &lt;b&gt;жирным&lt;/b&gt;.&lt;/p&gt;&lt;p&gt;Двойной шелчок или нажатие Return/Enter на предмете покажет его в окне предметов.&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../../finditemsdialog.cpp" line="341"/>
        <source>No items found</source>
        <translation>Предметов не найдено</translation>
    </message>
    <message>
        <location filename="../../finditemsdialog.cpp" line="381"/>
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
        <location filename="../../itemdatabase.cpp" line="25"/>
        <source>Reason: %1</source>
        <translation>Причина: %1</translation>
    </message>
    <message>
        <location filename="../../itemdatabase.cpp" line="31"/>
        <source>Error decrypting file &apos;%1&apos;</source>
        <translation>Ошибка при расшифровке файла &apos;%1&apos;</translation>
    </message>
    <message>
        <location filename="../../itemdatabase.cpp" line="67"/>
        <source>Items data not loaded.</source>
        <translation>Данные о предметах не загружены</translation>
    </message>
    <message>
        <location filename="../../itemdatabase.cpp" line="101"/>
        <source>Item types data not loaded.</source>
        <translation>Данные о типах предметов не загружены.</translation>
    </message>
    <message>
        <location filename="../../itemdatabase.cpp" line="123"/>
        <source>Properties data not loaded.</source>
        <translation>Данные о свойствах не загружены.</translation>
    </message>
    <message>
        <location filename="../../itemdatabase.cpp" line="166"/>
        <source>Sets data not loaded.</source>
        <translation>Данные о комплектах не загружены.</translation>
    </message>
    <message>
        <location filename="../../itemdatabase.cpp" line="195"/>
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
        <location filename="../../itemdatabase.cpp" line="427"/>
        <source>%1&apos;s </source>
        <comment>personalized name</comment>
        <translatorcomment>имя в именованном предмете</translatorcomment>
        <translation>(%1) </translation>
    </message>
    <message>
        <location filename="../../itemdatabase.cpp" line="461"/>
        <source>magic</source>
        <translation>магический</translation>
    </message>
    <message>
        <location filename="../../itemdatabase.cpp" line="464"/>
        <source>set</source>
        <translation>комплектный</translation>
    </message>
    <message>
        <location filename="../../itemdatabase.cpp" line="467"/>
        <source>rare</source>
        <translation>редкий</translation>
    </message>
    <message>
        <location filename="../../itemdatabase.cpp" line="470"/>
        <source>charm</source>
        <translation>талисман</translation>
    </message>
    <message>
        <location filename="../../itemdatabase.cpp" line="470"/>
        <source>unique</source>
        <translation>уникальный</translation>
    </message>
    <message>
        <location filename="../../itemdatabase.cpp" line="473"/>
        <source>crafted</source>
        <translation>крафтовый</translation>
    </message>
    <message>
        <location filename="../../itemdatabase.cpp" line="476"/>
        <source>honorific</source>
        <translation>почётный</translation>
    </message>
    <message>
        <location filename="../../itemdatabase.cpp" line="484"/>
        <source>runeword</source>
        <translation>рунное слово</translation>
    </message>
    <message>
        <location filename="../../itemdatabase.cpp" line="496"/>
        <source>ethereal</source>
        <translation>небесный</translation>
    </message>
    <message>
        <location filename="../../itemdatabase.cpp" line="555"/>
        <source>Error loading &apos;%1&apos;</source>
        <translation>Ошибка при загрузке &apos;%1&apos;</translation>
    </message>
    <message>
        <location filename="../../itemdatabase.cpp" line="555"/>
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
        <location filename="../../itemparser.cpp" line="131"/>
        <source>Item &apos;%1&apos; of unknown quality &apos;%2&apos; found!</source>
        <translation>Найден предмет &apos;%1&apos; неизвестного качества &apos;%2&apos;!</translation>
    </message>
    <message>
        <location filename="../../itemparser.cpp" line="188"/>
        <source>Veterans</source>
        <translation>Ветераны</translation>
    </message>
    <message>
        <location filename="../../itemparser.cpp" line="188"/>
        <source>Trophy&apos;d</source>
        <translation>С трофеем</translation>
    </message>
    <message>
        <location filename="../../itemparser.cpp" line="189"/>
        <source>Blessed</source>
        <translation>Улучшен</translation>
    </message>
    <message>
        <location filename="../../itemparser.cpp" line="180"/>
        <source>Error parsing item properties (status == failed), please report!</source>
        <translation>Ошибка чтения свойств (статус == фэил), пожалуйста сообщите!</translation>
    </message>
    <message>
        <location filename="../../itemparser.cpp" line="203"/>
        <source>Error parsing RW properties (status == failed), please report!</source>
        <translation>Ошибка чтения свойств РС (статус == фэил), пожалуйста сообщите!</translation>
    </message>
    <message>
        <location filename="../../itemparser.cpp" line="238"/>
        <source>Unknown RW, please report!</source>
        <translation>Неизвестное рунное слово, пожалуйста сообщите!</translation>
    </message>
    <message>
        <location filename="../../itemparser.cpp" line="242"/>
        <source>Unknown RW (no socketables detected), please report!</source>
        <translation>Неизвестное рунное слово (отсутствуют предметы в гнёздах), пожалуйста сообщите!</translation>
    </message>
    <message>
        <location filename="../../itemparser.cpp" line="344"/>
        <source>+%1 to %2 Skill Levels</source>
        <comment>+x to class skills</comment>
        <translatorcomment>+х к навыкам класса</translatorcomment>
        <translation>+%1 к Навыкам (%2)</translation>
    </message>
    <message>
        <location filename="../../itemparser.cpp" line="348"/>
        <source>+%1 to %2</source>
        <comment>oskill</comment>
        <translatorcomment>оскилл</translatorcomment>
        <translation>+%1 к %2</translation>
    </message>
    <message>
        <location filename="../../itemparser.cpp" line="350"/>
        <source>(%1 Only)</source>
        <comment>class-specific skill</comment>
        <translatorcomment>классовый навык</translatorcomment>
        <translation>(Только %1)</translation>
    </message>
    <message>
        <location filename="../../itemparser.cpp" line="354"/>
        <source>Level %1 %2 (%3/%4 Charges)</source>
        <translation>Уровень %1 %2 (%3/%4 Зарядов)</translation>
    </message>
    <message>
        <location filename="../../itemparser.cpp" line="374"/>
        <source>Error parsing item properties (exception == %1), please report!</source>
        <translation>Ошибка чтения свойств (exception == %1), пожалуйста сообщите!</translation>
    </message>
</context>
<context>
    <name>ItemsPropertiesSplitter</name>
    <message>
        <location filename="../../itemspropertiessplitter.cpp" line="46"/>
        <source>Page #</source>
        <translation>Стр. №</translation>
    </message>
    <message>
        <location filename="../../itemspropertiessplitter.cpp" line="549"/>
        <source>Arcane Shards</source>
        <translation>Тайные Осколки</translation>
    </message>
    <message>
        <location filename="../../itemspropertiessplitter.cpp" line="542"/>
        <source>Signet of Learning</source>
        <translation>Печать Обучения</translation>
    </message>
    <message>
        <location filename="../../itemspropertiessplitter.cpp" line="290"/>
        <source>Disenchant into</source>
        <translation>Расщепить на</translation>
    </message>
    <message>
        <location filename="../../itemspropertiessplitter.cpp" line="316"/>
        <source>All</source>
        <translation>Все</translation>
    </message>
    <message>
        <location filename="../../itemspropertiessplitter.cpp" line="330"/>
        <source>Remove Mystic Orbs</source>
        <translation>Удалить Мистические Сферы</translation>
    </message>
    <message>
        <location filename="../../itemspropertiessplitter.cpp" line="569"/>
        <source>Delete</source>
        <translation>Удалить</translation>
    </message>
    <message>
        <location filename="../../itemspropertiessplitter.cpp" line="466"/>
        <source>Are you sure you want to delete this item?</source>
        <translation>Вы точно хотите удалить этот предмет?</translation>
    </message>
    <message>
        <location filename="../../itemspropertiessplitter.cpp" line="470"/>
        <source>Cube is not empty. Do you really want to delete it?
Note: items inside will be preserved. You can recover them by getting new Cube.</source>
        <translation>Куб не пуст. Вы точно хотите его удалить?
Примечание: содержимое сохранится. Вы сможете восстановить его при получении нового Куба.</translation>
    </message>
</context>
<context>
    <name>ItemsViewerDialog</name>
    <message>
        <location filename="../../itemsviewerdialog.cpp" line="196"/>
        <source>Gear</source>
        <translation>Экипировка</translation>
    </message>
    <message>
        <location filename="../../itemsviewerdialog.cpp" line="196"/>
        <source>Inventory</source>
        <translation>Инвентарь</translation>
    </message>
    <message>
        <location filename="../../itemsviewerdialog.cpp" line="196"/>
        <source>Cube</source>
        <translation>Куб</translation>
    </message>
    <message>
        <location filename="../../itemsviewerdialog.cpp" line="196"/>
        <source>Stash</source>
        <translation>Сундук</translation>
    </message>
    <message>
        <location filename="../../itemsviewerdialog.cpp" line="196"/>
        <source>Personal Stash</source>
        <translation>Личный Сундук</translation>
    </message>
    <message>
        <location filename="../../itemsviewerdialog.cpp" line="196"/>
        <source>Shared Stash</source>
        <translation>Общий Сундук</translation>
    </message>
    <message>
        <location filename="../../itemsviewerdialog.cpp" line="196"/>
        <source>Hardcore Stash</source>
        <translation>Смертный Сундук</translation>
    </message>
    <message>
        <location filename="../../itemsviewerdialog.cpp" line="213"/>
        <source>Items viewer (items total: %1)</source>
        <translation>Просмотр предметов (всего предметов: %1)</translation>
    </message>
</context>
<context>
    <name>Language</name>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1035"/>
        <source>English</source>
        <comment>Your language name</comment>
        <translation>Русский</translation>
    </message>
</context>
<context>
    <name>MedianXLOfflineTools</name>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="74"/>
        <source>1337 hacker detected! Please, play legit.</source>
        <translation>Кулхацкер детектед! Пожалуйста, играйте честно.</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="75"/>
        <source>Max: %1</source>
        <translation>Максимум: %1</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="75"/>
        <source>Min: %1</source>
        <translation>Минимум: %1</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="75"/>
        <source>Invested: %1</source>
        <translation>Вложено: %1</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="211"/>
        <source>Language will be changed next time you run the application</source>
        <translation>Язык изменится при следующем запуске программы</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="229"/>
        <source>Load Character</source>
        <translation>Загрузить Персонажа</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="231"/>
        <source>Character loaded</source>
        <translation>Персонаж загружен</translation>
    </message>
    <message id="true">
        <location filename="../../medianxlofflinetools.cpp" line="242"/>
        <source>Character reloaded</source>
        <translation>Персонаж перезагружен</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="539"/>
        <source>File &apos;%1&apos; successfully saved!</source>
        <translation>Файл &apos;%1&apos; успешно сохранён!</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="884"/>
        <source>About %1</source>
        <translation>О %1</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="888"/>
        <source>Released: %1</source>
        <translation>Выпущено: %1</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="891"/>
        <source>&lt;i&gt;Author:&lt;/i&gt; Filipenkov Andrey (&lt;b&gt;kambala&lt;/b&gt;)</source>
        <translation>&lt;i&gt;Автор:&lt;/i&gt; Филипенков Андрей (&lt;b&gt;kambala&lt;/b&gt;)</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="893"/>
        <source>&lt;b&gt;Credits:&lt;/b&gt;&lt;ul&gt;&lt;li&gt;&lt;a href=&quot;http://modsbylaz.hugelaser.com/&quot;&gt;BrotherLaz&lt;/a&gt; for this awesome mod&lt;/li&gt;&lt;li&gt;&lt;a href=&quot;http://modsbylaz.14.forumer.com/profile.php?mode=viewprofile&amp;u=33805&quot;&gt;grig&lt;/a&gt; for the Perl source of &lt;a href=&quot;http://grig.vlexofree.com/&quot;&gt;Median XL Online Tools&lt;/a&gt; and tips&lt;/li&gt;&lt;li&gt;&lt;a href=&quot;http://phrozenkeep.hugelaser.com/index.php?ind=reviews&amp;op=section_view&amp;idev=4&quot;&gt;Phrozen Keep File Guides&lt;/a&gt; for tons of useful information on txt sources&lt;/li&gt;&lt;li&gt;&lt;a href=&quot;http://modsbylaz.14.forumer.com/profile.php?mode=viewprofile&amp;u=44046&quot;&gt;FixeR&lt;/a&gt;, &lt;a href=&quot;http://forum.worldofplayers.ru/member.php?u=84592&quot;&gt;Zelgadiss&lt;/a&gt; and &lt;a href=&quot;http://modsbylaz.14.forumer.com/profile.php?mode=viewprofile&amp;u=44840&quot;&gt;moonra&lt;/a&gt; for intensive testing and tips on GUI &amp; functionality&lt;/li&gt;&lt;/ul&gt;</source>
        <translation>&lt;b&gt;Благодарности:&lt;/b&gt;&lt;ul&gt;&lt;li&gt;&lt;a href=&quot;http://modsbylaz.hugelaser.com/&quot;&gt;BrotherLaz&apos;у&lt;/a&gt; за этот шикарный мод&lt;/li&gt;&lt;li&gt;&lt;a href=&quot;http://modsbylaz.14.forumer.com/profile.php?mode=viewprofile&amp;u=33805&quot;&gt;grig&apos;у&lt;/a&gt; за Перловские исходники его &lt;a href=&quot;http://grig.vlexofree.com/&quot;&gt;Median XL Online Tools&lt;/a&gt; и за помощь&lt;/li&gt;&lt;li&gt;&lt;a href=&quot;http://phrozenkeep.hugelaser.com/index.php?ind=reviews&amp;op=section_view&amp;idev=4&quot;&gt;Phrozen Keep Гайдам&lt;/a&gt; за кучу полезной информации по текстовикам&lt;/li&gt;&lt;li&gt;&lt;a href=&quot;http://modsbylaz.14.forumer.com/profile.php?mode=viewprofile&amp;u=44046&quot;&gt;FixeR&apos;у&lt;/a&gt;, &lt;a href=&quot;http://forum.worldofplayers.ru/member.php?u=84592&quot;&gt;Zelgadiss&apos;у&lt;/a&gt; и &lt;a href=&quot;http://modsbylaz.14.forumer.com/profile.php?mode=viewprofile&amp;u=44840&quot;&gt;moonra&lt;/a&gt; за интенсивное тестирование и советы по ГУИ и функциональности&lt;/li&gt;&lt;/ul&gt;</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1022"/>
        <source>&amp;Language</source>
        <comment>Language menu</comment>
        <translation>&amp;Язык</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1107"/>
        <location filename="../../medianxlofflinetools.cpp" line="1146"/>
        <source>Name</source>
        <translation>Имя</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1111"/>
        <source>Class</source>
        <translation>Класс</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1113"/>
        <location filename="../../medianxlofflinetools.cpp" line="1144"/>
        <source>Level</source>
        <translation>Уровень</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1115"/>
        <source>Title</source>
        <comment>Character title - Slayer/Champion/etc.</comment>
        <translation>Титул</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1142"/>
        <source>Type</source>
        <translation>Тип</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1160"/>
        <source>Inventory Gold</source>
        <translation>Золота в Инвентаре</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1163"/>
        <source>Stash Gold</source>
        <translation>Золота в Сундуке</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1173"/>
        <source>Strength</source>
        <translation>Сила</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1174"/>
        <source>Dexterity</source>
        <translation>Ловкость</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1175"/>
        <source>Vitality</source>
        <translation>Живучесть</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1176"/>
        <source>Energy</source>
        <translation>Энергиия</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="447"/>
        <location filename="../../medianxlofflinetools.cpp" line="545"/>
        <source>Error creating file &apos;%1&apos;</source>
        <translation>Ошибка при создании файла &apos;%1&apos;</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="98"/>
        <source>%1 is not associated with %2 files.

Do you want to do it?</source>
        <translation>%1 не ассоциирована с %2 файлами.

Вы хотите это сделать?</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="2289"/>
        <source>Application is default already</source>
        <translation>Приложение уже является по умолчанию</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="150"/>
        <source>&apos;%1&apos; files are not supported</source>
        <comment>param is file extension</comment>
        <translation>&apos;%1&apos; файлы не поддерживаются</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="76"/>
        <source>&lt;a href=&quot;http://modsbylaz.14.forumer.com/viewtopic.php?t=23147&quot;&gt;Official Median XL Forum thread&lt;/a&gt;&lt;br&gt;&lt;a href=&quot;http://forum.worldofplayers.ru/showthread.php?t=34489&quot;&gt;Official Russian Median XL Forum thread&lt;/a&gt;</source>
        <translation>&lt;a href=&quot;http://modsbylaz.14.forumer.com/viewtopic.php?t=23147&quot;&gt;Тема на официальном форуме Median XL&lt;/a&gt;&lt;br&gt;&lt;a href=&quot;http://forum.worldofplayers.ru/showthread.php?t=34489&quot;&gt;Тема на официальном русском форуме Median XL&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="229"/>
        <source>Diablo 2 Save Files</source>
        <translation>Файлы Сохранений Diablo 2</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="518"/>
        <source>Error removing file &apos;%1&apos;</source>
        <translation>Ошибка при удалении файла &apos;%1&apos;</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="523"/>
        <source>Error renaming file &apos;%1&apos;</source>
        <translation>Ошибка при переименовании файла &apos;%1&apos;</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="542"/>
        <source>Error writing file &apos;%1&apos;</source>
        <translation>Ошибка при записи файла &apos;%1&apos;</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="822"/>
        <source>You have no free space in inventory and stash to store the Cube</source>
        <translation>У вас нет свободного места в инвентаре и сундуке для сохранения Куба</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="850"/>
        <source>Cube has been stored in %1 at (%2,%3)</source>
        <translation>Куб сохранён в %1 в клетку (%2,%3)</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="861"/>
        <source>Are you sure you want to disable automatic backups? Then don&apos;t blame me if your character gets corrupted.</source>
        <translation>Вы точно хотите отключить автоматическое создание резервных копий? Тогда не вините меня если вдруг ваш персонаж сломается.</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="951"/>
        <source>Experience table data not loaded.</source>
        <translation>Таблица опыта не загружена.</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="965"/>
        <source>Mercenary names not loaded.</source>
        <translation>Имена наёмников не загружены.</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="987"/>
        <source>Base stats data not loaded, using predefined one.</source>
        <translation>Данные о начальных атрибутах не найдены, используются предопределённые.</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1166"/>
        <source>Free Skills</source>
        <translation>Невложеных Навыков</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1169"/>
        <source>Signets of Skill</source>
        <translation>Печатей Навыка</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1182"/>
        <source>Free Stats</source>
        <translation>Невложеных Атрибутов</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1185"/>
        <source>Signets of Learning</source>
        <translation>Печатей Обучения</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1191"/>
        <source>Quests</source>
        <translation>Квесты</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1196"/>
        <source>Hatred</source>
        <translation>Ненависть</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1196"/>
        <source>Terror</source>
        <translation>Ужас</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1196"/>
        <source>Destruction</source>
        <translation>Разрушение</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1202"/>
        <source>Den of Evil</source>
        <translation>Логово Зла</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1203"/>
        <source>Radament</source>
        <translation>Радамент</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1204"/>
        <source>Izual</source>
        <translation>Изуал</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1205"/>
        <source>Lam Esen&apos;s Tome</source>
        <translation>Том Лам Эзена</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1441"/>
        <source>Error opening file &apos;%1&apos;</source>
        <translation>Ошибка при открытии файла &apos;%1&apos;</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1455"/>
        <source>Wrong file signature: should be 0x%1, got 0x%2.</source>
        <translation>Неверная сигнатура файла: должна быть 0x%1, а тут - 0x%2.</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1464"/>
        <source>Character checksum doesn&apos;t match. Looks like it&apos;s corrupted.</source>
        <translation>Контрольная сумма персонажа не совпадает. Похоже, он повреждён.</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1482"/>
        <source>This is not Expansion character.</source>
        <translation>Это не персонаж Расширения.</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1490"/>
        <source>Wrong class value: got %1</source>
        <translation>Некорректное значение класса: %1</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1497"/>
        <source>Wrong progression value: got %1</source>
        <translation>Некорректное значение прогресса: %1</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1504"/>
        <source>Wrong level: got %1</source>
        <translation>Некорректный уровень %1</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1531"/>
        <source>Wrong mercenary code: got %1</source>
        <translation>Некорректный код наёмника: %1</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1551"/>
        <source>Quests data not found!</source>
        <translation>Данные о квестах не найдены!</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1567"/>
        <source>Waypoint data not found!</source>
        <translation>Данные о телепортах не найдены!</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1594"/>
        <source>NPC data not found!</source>
        <translation>Данные о NPC не найдены!</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1601"/>
        <source>Stats data not found!</source>
        <translation>Данные о статистиках не найдены!</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1610"/>
        <source>Skills data not found!</source>
        <translation>Данные о навыках не найдены!</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1650"/>
        <source>Level in statistics (%1) isn&apos;t equal the one in header (%2).</source>
        <translation>Уровень в статистиках (%1) - не такой, как в заголовке (%2).</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1685"/>
        <source>Stats data is corrupted!</source>
        <translation>Данные о статистиках повреждены!</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1746"/>
        <source>Items data not found!</source>
        <translation>Данные о предметах не найдены!</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1763"/>
        <source>Corrupted item detected in %1 at (%2,%3)</source>
        <translation>%1 содержит испорченный предмет в клетке (%2,%3)</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1799"/>
        <source>Items list doesn&apos;t have a terminator!</source>
        <translation>У списка вещей нету завершающего символа!</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1808"/>
        <source>Mercenary items section not found!</source>
        <translation>Секция с вещами наёмника не найдена!</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1816"/>
        <source>Save file is not terminated correctly!</source>
        <translation>Файл сохранения завершён некорректно!</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1823"/>
        <source>Mercenary items data is corrupted!</source>
        <translation>Данные о вещах наёмника повреждены!</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="2474"/>
        <source>Character has been modified.</source>
        <translation>Персонаж был изменён.</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="2475"/>
        <source>Do you want to save your changes?</source>
        <translation>Вы хотите сохранить изменения?</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="2514"/>
        <source>New version &lt;b&gt;%1&lt;/b&gt; is available!</source>
        <translation>Доступна новая версия &lt;b&gt;%1&lt;/b&gt;!</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="2516"/>
        <source>You have the latest version</source>
        <translation>У вас самая свежая версия</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="2524"/>
        <source>Error contacting update server</source>
        <translation>Невозможно связаться с сервером обновлений</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1775"/>
        <source>100% avoid is kewl</source>
        <translation>Ололо 100% авоид</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1643"/>
        <source>Unknown statistic code found: %1. This is not %2 character.</source>
        <comment>second param is mod name</comment>
        <translatorcomment>второй параметр - это название мода</translatorcomment>
        <translation>Найден неизвестный код статистики: %1. Этот персонаж не из %2.</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1777"/>
        <source>well, you have %1% actually</source>
        <comment>avoid</comment>
        <translation>ну, вообще-то у вас %1%</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1934"/>
        <source>Error opening PlugY stash &apos;%1&apos;</source>
        <translation>Ошибка при открытии сундука Плаги&apos;%1&apos;</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1952"/>
        <source>PlugY stash &apos;%1&apos; has wrong header</source>
        <translation>У сундука Плаги &apos;%1&apos; неправильный хедер</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1973"/>
        <source>Page %1 of &apos;%2&apos; has wrong PlugY header</source>
        <translation>У %1-й страницы &apos;%2&apos; неправильный хедер Плаги</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1979"/>
        <source>Page %1 of &apos;%2&apos; has wrong item header</source>
        <translation>У %1-й страницы &apos;%2&apos; неправильный хедер предмета</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="1994"/>
        <source>Corrupted item detected in %1 on page %4 at (%2,%3)</source>
        <translation>%1 содержит испорченный предмет на странице %4 в клетке (%2,%3)</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="2108"/>
        <source>Shared: %1</source>
        <comment>amount of gold in shared stash</comment>
        <translatorcomment>количество золота в общем сундуке</translatorcomment>
        <translation>Общий Сундук: %1</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="2174"/>
        <source>DEAD</source>
        <comment>HC character is dead</comment>
        <translation>ТРУП</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="2392"/>
        <source>Stats string is not byte aligned!</source>
        <translation>Строка статистик не выровнена по границе байта!</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="2443"/>
        <source>Error removing old backup &apos;%1&apos;</source>
        <translation>Ошибка при удалении старой резервной копии &apos;%1&apos;</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="2447"/>
        <source>Error creating backup of &apos;%1&apos;</source>
        <translation>Ошибка при создании резервной копии &apos;%1&apos;</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.cpp" line="2462"/>
        <source>Reason: %1</source>
        <comment>error with file</comment>
        <translation>Причина: %1</translation>
    </message>
</context>
<context>
    <name>MedianXLOfflineToolsClass</name>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="37"/>
        <source>Stats</source>
        <translation>Статистика</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="136"/>
        <source>Life</source>
        <translation>Жизнь</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="141"/>
        <source>Mana</source>
        <translation>Мана</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="146"/>
        <source>Stamina</source>
        <translation>Выносливость</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="151"/>
        <source>Current</source>
        <extracomment>life/mana/stamina</extracomment>
        <translation>Текущая</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="156"/>
        <source>Base</source>
        <extracomment>life/mana/stamina</extracomment>
        <translation>Базовая</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="222"/>
        <location filename="../../medianxlofflinetools.ui" line="836"/>
        <source>Respec Skills</source>
        <translation>Сбросить Навыки</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="235"/>
        <location filename="../../medianxlofflinetools.ui" line="850"/>
        <source>Activate Waypoints</source>
        <translation>Активировать Телепорты</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="312"/>
        <location filename="../../medianxlofflinetools.ui" line="822"/>
        <source>Respec Stats</source>
        <translation>Сбросить Атрибуты</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="332"/>
        <source>Character</source>
        <translation>Персонаж</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="344"/>
        <location filename="../../medianxlofflinetools.ui" line="861"/>
        <source>Rename</source>
        <translation>Переименовать</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="488"/>
        <location filename="../../medianxlofflinetools.ui" line="1050"/>
        <location filename="../../medianxlofflinetools.ui" line="1053"/>
        <source>Hardcore</source>
        <translation>Смертный</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="500"/>
        <location filename="../../medianxlofflinetools.ui" line="893"/>
        <source>Convert to Softcore</source>
        <translation>Сделать не Смертным</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="516"/>
        <location filename="../../medianxlofflinetools.ui" line="901"/>
        <source>Resurrect</source>
        <translation>Оживить</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="540"/>
        <source>Mercenary</source>
        <translation>Наёмник</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="597"/>
        <source>&amp;File</source>
        <translation>&amp;Файл</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="604"/>
        <source>Recent Characters</source>
        <translation>Последние Персонажи</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="617"/>
        <source>&amp;Help</source>
        <translation>&amp;Справка</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="626"/>
        <source>&amp;Edit</source>
        <translation>&amp;Правка</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="643"/>
        <source>Auto-open shared stashes</source>
        <translation>Авто-открытие общих сундуков</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="671"/>
        <source>Go to page</source>
        <translation>Перейти на страницу</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="710"/>
        <location filename="../../medianxlofflinetools.ui" line="909"/>
        <source>Toolbar</source>
        <translation>Панель Инструментов</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="743"/>
        <source>L&amp;oad Character...</source>
        <translation>&amp;Загрузить Персонажа...</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="746"/>
        <source>Ctrl+O</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="761"/>
        <source>&amp;Save Character</source>
        <translation>&amp;Сохранить Персонажа</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="764"/>
        <source>Ctrl+S</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="772"/>
        <source>Exit</source>
        <translation>Выход</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="784"/>
        <source>About</source>
        <translation>О программе</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="825"/>
        <source>F5</source>
        <translation>F5</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="839"/>
        <source>F6</source>
        <translation>F6</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="853"/>
        <source>F7</source>
        <translation>F7</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="921"/>
        <source>Show items</source>
        <translation>Показать предметы</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="924"/>
        <source>Show Items</source>
        <translation>Показать Предметы</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="927"/>
        <source>Ctrl+I</source>
        <translation>Ctrl+I</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="941"/>
        <source>Load last used character</source>
        <translation>Загружать последнего персонажа</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="949"/>
        <source>Give me the Cube</source>
        <translation>Хочу Куб</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="952"/>
        <source>Present your character a new Horadric Cube</source>
        <translation>Подарите вашему любимчику новый Хорадрический Куб</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="960"/>
        <source>Auto-open items window</source>
        <translation>Авто-открытие окна предметов</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="963"/>
        <source>Open items window when a character is loaded</source>
        <translation>Открывать окно предметов сразу после загрузки персонажа</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="975"/>
        <source>Find...</source>
        <translation>Поиск...</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="978"/>
        <source>Find Items</source>
        <translation>Поиск Предметов</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="981"/>
        <source>Ctrl+F</source>
        <translation>Ctrl+F</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="992"/>
        <source>Find next</source>
        <translation>Найти следующий</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="1003"/>
        <source>Find previous</source>
        <translation>Найти предыдущий </translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="1017"/>
        <source>Reload shared stashes when loading a character (may be slow)</source>
        <translation>Перезагружать общие сундуки при загрузке персонажа (может быть медленно)</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="1206"/>
        <source>Skillplan</source>
        <translation>План Навыков</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="1209"/>
        <source>Generate a link for Tub&apos;s Skillpoint Planner</source>
        <translation>Генерирует ссылку для Планировщика Навыков Tub&apos;а</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="1212"/>
        <source>Ctrl+P</source>
        <translation>Ctrl+P</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="1220"/>
        <source>Character Info</source>
        <translation>Информация о Персонаже</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="1231"/>
        <source>Check file associations on start</source>
        <translation>Проверять ассоциации файлов при запуске</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="1236"/>
        <source>Associate with .d2s files</source>
        <translation>Ассоциировать с .d2s файлами</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="1247"/>
        <source>Check for update on start</source>
        <translation>Проверять обновление при запуске</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="1252"/>
        <source>Check for Update</source>
        <translation>Проверить Обновление</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="1014"/>
        <source>Always reload shared stashes</source>
        <translation>Всегда перезагружать общие сундуки</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="639"/>
        <source>&amp;Options</source>
        <translation>&amp;Настройки</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="664"/>
        <source>&amp;Items</source>
        <translation>Пре&amp;дметы</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="693"/>
        <source>E&amp;xport</source>
        <translation>&amp;Экспорт</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="1028"/>
        <source>Personal</source>
        <translation>Личный</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="1039"/>
        <source>Shared</source>
        <translation>Общий</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="1064"/>
        <source>Make backups before saving</source>
        <translation>Создавать резервные копии перед сохранением</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="1075"/>
        <source>Warn when new name has color</source>
        <translation>Предупреждать если новое имя содержит цвет</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="1078"/>
        <source>Show confirmation dialog when using color in new name</source>
        <translation>Спрашивать подтверждение при попытке сохранения цветного имени</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="1087"/>
        <source>First</source>
        <translation>Первая</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="1102"/>
        <source>Previous 100</source>
        <translation>Назад на 100</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="1117"/>
        <source>Previous 10</source>
        <translation>Назад на 10</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="1132"/>
        <source>Previous</source>
        <translation>Назад</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="1147"/>
        <source>Next</source>
        <translation>Вперёд</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="1162"/>
        <source>Next 10</source>
        <translation>Вперёд на 10</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="1177"/>
        <source>Next 100</source>
        <translation>Вперёд на 100</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="1192"/>
        <source>Last</source>
        <translation>Последняя</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="787"/>
        <source>F1</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="802"/>
        <location filename="../../medianxlofflinetools.ui" line="805"/>
        <location filename="../../medianxlofflinetools.ui" line="808"/>
        <source>About Qt</source>
        <translation>О Qt</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="864"/>
        <source>Alt+N</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="876"/>
        <source>&amp;Reload Character</source>
        <translation>&amp;Перезагрузить Персонажа</translation>
    </message>
    <message>
        <location filename="../../medianxlofflinetools.ui" line="879"/>
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
        <location filename="../../propertiesdisplaymanager.cpp" line="57"/>
        <source>Defense: %1</source>
        <translation>Защита: %1</translation>
    </message>
    <message>
        <location filename="../../propertiesdisplaymanager.cpp" line="65"/>
        <source>Durability</source>
        <translation>Прочность</translation>
    </message>
    <message>
        <location filename="../../propertiesdisplaymanager.cpp" line="69"/>
        <source>%1 of %2</source>
        <comment>durability</comment>
        <translatorcomment>прочность</translatorcomment>
        <translation>%1 из %2</translation>
    </message>
    <message>
        <location filename="../../propertiesdisplaymanager.cpp" line="74"/>
        <source>Quantity: %1</source>
        <translation>Количество: %1</translation>
    </message>
    <message>
        <location filename="../../propertiesdisplaymanager.cpp" line="76"/>
        <source>(%1 Only)</source>
        <comment>class-specific item</comment>
        <translatorcomment>классовый предмет</translatorcomment>
        <translation>(Только %1)</translation>
    </message>
    <message>
        <location filename="../../propertiesdisplaymanager.cpp" line="100"/>
        <source>Required Level: %1</source>
        <translation>Нужен Уровень: %1</translation>
    </message>
    <message>
        <location filename="../../propertiesdisplaymanager.cpp" line="115"/>
        <source>[Unidentified]</source>
        <translation>[Неидентифицировано]</translation>
    </message>
    <message>
        <location filename="../../propertiesdisplaymanager.cpp" line="128"/>
        <source>Armor</source>
        <translation>Броня</translation>
    </message>
    <message>
        <location filename="../../propertiesdisplaymanager.cpp" line="128"/>
        <source>Shield</source>
        <translation>Щит</translation>
    </message>
    <message>
        <location filename="../../propertiesdisplaymanager.cpp" line="128"/>
        <source>Weapon</source>
        <translation>Оружие</translation>
    </message>
    <message>
        <location filename="../../propertiesdisplaymanager.cpp" line="153"/>
        <source>+50% Damage to Undead</source>
        <translation>+50% Урон Нежити</translation>
    </message>
    <message>
        <location filename="../../propertiesdisplaymanager.cpp" line="155"/>
        <source>Socketed: (%1), Inserted: (%2)</source>
        <translation>Гнёзд: (%1), Вставлено: (%2)</translation>
    </message>
    <message>
        <location filename="../../propertiesdisplaymanager.cpp" line="157"/>
        <source>Ethereal (Cannot be Repaired)</source>
        <translation>Неземной (Нельзя Починить)</translation>
    </message>
    <message>
        <location filename="../../propertiesdisplaymanager.cpp" line="247"/>
        <source>Based on</source>
        <comment>&apos;based on level&apos; property; translate only if Median XL is translated into your language! (i.e. there&apos;s localized data in Resources/data/&lt;language&gt;)</comment>
        <translatorcomment>свойство &quot;основано на уровне&quot;; переводить только если Median XL переведён на ваш язык! (т.е. есть локализованные данные в Resources/data/&lt;язык&gt;)</translatorcomment>
        <translation>Основано на</translation>
    </message>
    <message>
        <location filename="../../propertiesdisplaymanager.cpp" line="291"/>
        <source>Repairs 1 Durability in %1 Seconds</source>
        <translation>Восстанавливает Единицу Прочности за %1 Секунд</translation>
    </message>
    <message>
        <location filename="../../propertiesdisplaymanager.cpp" line="308"/>
        <source>[special case %1, please report] %2 &apos;%3&apos; (id %4)</source>
        <translation>[особый случай %1, пожалуйста сообщите] %2 &apos;%3&apos; (id %4)</translation>
    </message>
    <message>
        <location filename="../../propertiesdisplaymanager.cpp" line="334"/>
        <source>They have Windows in Hell</source>
        <translation>У них есть Окна в Аду (Больше не существующая мини-игра)</translation>
    </message>
    <message>
        <location filename="../../propertiesdisplaymanager.cpp" line="337"/>
        <source>Mirror Mirror</source>
        <translation>Зеркало Зеркало (мини-игра 3)</translation>
    </message>
    <message>
        <location filename="../../propertiesdisplaymanager.cpp" line="340"/>
        <source>Countess</source>
        <translation>Графиня (мини-игра 1)</translation>
    </message>
    <message>
        <location filename="../../propertiesdisplaymanager.cpp" line="343"/>
        <source>Level Challenge 2</source>
        <translation>Уровневое Испытание 2</translation>
    </message>
    <message>
        <location filename="../../propertiesdisplaymanager.cpp" line="346"/>
        <source>Crowned</source>
        <translation>Коронация (мини-игра 2)</translation>
    </message>
    <message>
        <location filename="../../propertiesdisplaymanager.cpp" line="349"/>
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
        <location filename="../../propertiesviewerwidget.cpp" line="90"/>
        <source>Item Level: %1</source>
        <translation>Уровень Предмета: %1</translation>
    </message>
    <message>
        <location filename="../../propertiesviewerwidget.cpp" line="113"/>
        <source>Defense: %1</source>
        <translation>Защита: %1</translation>
    </message>
    <message>
        <location filename="../../propertiesviewerwidget.cpp" line="122"/>
        <source>Durability</source>
        <translation>Прочность</translation>
    </message>
    <message>
        <location filename="../../propertiesviewerwidget.cpp" line="126"/>
        <source>%1 of %2</source>
        <comment>durability</comment>
        <translation>%1 из %2</translation>
    </message>
    <message>
        <location filename="../../propertiesviewerwidget.cpp" line="132"/>
        <source>Quantity: %1</source>
        <translation>Количество: %1</translation>
    </message>
    <message>
        <location filename="../../propertiesviewerwidget.cpp" line="135"/>
        <source>(%1 Only)</source>
        <comment>class-specific item</comment>
        <translation>(Только %1)</translation>
    </message>
    <message>
        <location filename="../../propertiesviewerwidget.cpp" line="168"/>
        <source>Required Level: %1</source>
        <translation>Нужен Уровень: %1</translation>
    </message>
    <message>
        <location filename="../../propertiesviewerwidget.cpp" line="183"/>
        <source>[Unidentified]</source>
        <translation>[Неидентифицировано]</translation>
    </message>
    <message>
        <location filename="../../propertiesviewerwidget.cpp" line="188"/>
        <source>Armor</source>
        <translation>Броня</translation>
    </message>
    <message>
        <location filename="../../propertiesviewerwidget.cpp" line="188"/>
        <source>Shield</source>
        <translation>Щит</translation>
    </message>
    <message>
        <location filename="../../propertiesviewerwidget.cpp" line="188"/>
        <source>Weapon</source>
        <translation>Оружие</translation>
    </message>
    <message>
        <location filename="../../propertiesviewerwidget.cpp" line="202"/>
        <source>+50% Damage to Undead</source>
        <translation>+50% Урон Нежити</translation>
    </message>
    <message>
        <location filename="../../propertiesviewerwidget.cpp" line="204"/>
        <source>Socketed: (%1), Inserted: (%2)</source>
        <translation>Гнёзд: (%1), Вставлено: (%2)</translation>
    </message>
    <message>
        <location filename="../../propertiesviewerwidget.cpp" line="206"/>
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
        <location filename="../../qd2charrenamer.cpp" line="135"/>
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
<context>
    <name>SkillplanDialog</name>
    <message>
        <location filename="../../skillplandialog.ui" line="17"/>
        <location filename="../../skillplandialog.cpp" line="146"/>
        <source>Skillplan</source>
        <translation>План Навыков</translation>
    </message>
    <message>
        <location filename="../../skillplandialog.ui" line="26"/>
        <source>Use available (otherwise max possible is used)</source>
        <translation>Использовать доступные (иначе используются максимально возможные)</translation>
    </message>
    <message>
        <location filename="../../skillplandialog.ui" line="38"/>
        <source>Skill quests</source>
        <translation>Квесты на навыки</translation>
    </message>
    <message>
        <location filename="../../skillplandialog.ui" line="58"/>
        <source>Charms</source>
        <translation>Талисманы</translation>
    </message>
    <message>
        <location filename="../../skillplandialog.ui" line="78"/>
        <source>Minigames</source>
        <translation>Мини-игры</translation>
    </message>
    <message>
        <location filename="../../skillplandialog.ui" line="98"/>
        <source>Signets of Skill</source>
        <translation>Печати Навыка</translation>
    </message>
    <message>
        <location filename="../../skillplandialog.ui" line="118"/>
        <source>Items</source>
        <translation>Предметы</translation>
    </message>
    <message>
        <location filename="../../skillplandialog.ui" line="156"/>
        <source>Copy BBCode</source>
        <translation>Копировать BBCode</translation>
    </message>
    <message>
        <location filename="../../skillplandialog.ui" line="170"/>
        <source>Copy HTML</source>
        <translation>Копировать HTML</translation>
    </message>
    <message>
        <location filename="../../skillplandialog.cpp" line="157"/>
        <source>If you check a checkbox, then corresponding value is taken from your character. If you don&apos;t, then maximum possible value is used.

To copy simple link: right-click the blue-highlighted link and click on the menu command.</source>
        <translation>Если вы поставите галочку, тогда соответствующая величина берётся с вашего персонажа. Если же не поставите, тогда используется максимально возможная.

Для копирования обычной ссылки: нажмите правой кнопкой мыши на подсвеченную голубым ссылку и нажмите на команду появившегося меню.</translation>
    </message>
</context>
</TS>
