#include "itemparser.h"
#include "itemdatabase.h"
#include "reversebitreader.h"
#include "helpers.h"

#include <QFile>
#include <QDataStream>

#ifndef QT_NO_DEBUG
#include <QDebug>
#endif


const QByteArray ItemParser::itemHeader("JM");
const QByteArray ItemParser::plugyPageHeader("ST");

ItemInfo *ItemParser::parseItem(QDataStream &inputDataStream, const QByteArray &bytes)
{
	ItemInfo *item = 0;
	bool ok = true;
	int attempt = 0, searchEndOffset = 0;
	// loop tries maximum 5 times to read past JM in case it's a part of item
	do
	{
		inputDataStream.skipRawData(2); // JM
		int itemStartOffset = inputDataStream.device()->pos();
		if (!searchEndOffset)
			searchEndOffset = itemStartOffset;
		int nextItemOffset = bytes.indexOf(itemHeader, searchEndOffset);
		if (nextItemOffset == -1)
			nextItemOffset = bytes.size();
		else if (bytes.mid(nextItemOffset - 3, 2) == plugyPageHeader) // for plugy stashes
			nextItemOffset -= 3;
		int itemSize = nextItemOffset - itemStartOffset;

		QString itemBitData;
		itemBitData.reserve(itemSize * 8);
		for (int i = 0; i < itemSize; ++i)
		{
			quint8 aByte;
			inputDataStream >> aByte;
			itemBitData.prepend(binaryStringFromNumber(aByte));
		}
		//    QString result;
		//    QTextStream outStream(&result);
		ReverseBitReader bitReader(itemBitData);
		if (item)
			delete item;
		item = new ItemInfo(bitReader.notReadBits()/*, itemStartOffset, itemSize*/);
		item->isQuest = bitReader.readBool();
		//outStream << "is quest " << bitReader.readBool() << '\n';
		bitReader.skip(3);
		//outStream << "unk" << bitReader.readNumber(3) << '\n';
		item->isIdentified = bitReader.readBool();
		//outStream << "is identified " << isIdentified << '\n';
		bitReader.skip(5);
		//outStream << "unk" << bitReader.readNumber(5) << '\n';
		bitReader.skip();
		//outStream << "is duped " << bitReader.readBool() << '\n';
		item->isSocketed = bitReader.readBool();
		//outStream << "is socketed " << isSocketed << '\n';
		bitReader.skip(2);
		//outStream << "unk" << bitReader.readNumber(2) << '\n';
		//outStream << "is illegal equip " << bitReader.readBool() << '\n';
		bitReader.skip(2);
		//outStream << "unk" << bitReader.readBool() << '\n';
		item->isEar = bitReader.readBool();
		//outStream << "is ear " << isEar << '\n';
		item->isStarter = bitReader.readBool();
		//outStream << "is newbie " << bitReader.readBool() << '\n';
		bitReader.skip(2);
		//outStream << "unk" << bitReader.readNumber(2) << '\n';
		bitReader.skip();
		//outStream << "unk" << bitReader.readBool() << '\n';
		item->isExtended = !bitReader.readBool();
		//outStream << "is extended " << isExtended << '\n';
		item->isEthereal = bitReader.readBool();
		//outStream << "is ethereal " << isEthereal << '\n';
		bitReader.skip();
		item->isPersonalized = bitReader.readBool();
		//outStream << "is personalized " << isPersonalized << '\n';
		bitReader.skip();
		//outStream << "unk" << bitReader.readBool() << '\n';
		item->isRW = bitReader.readBool();
		//outStream << "is RW " << isRW << '\n';
		bitReader.skip(5);
		//outStream << "unk" << bitReader.readNumber(5) << '\n';
		bitReader.skip(8);
		//outStream << "version" << bitReader.readNumber(8) << '\n'; // should be 101
		bitReader.skip(2);
		//outStream << "unk" << bitReader.readNumber(2) << '\n';
		item->location = bitReader.readNumber(3);
		//outStream << "location " << location << '\n';
		item->whereEquipped = bitReader.readNumber(4);
		//    if (itemInfo->location == 1) // gear
		//        outStream << "where equipped " << whereEquipped << '\n';
		item->column = bitReader.readNumber(4);
		item->row = bitReader.readNumber(4);
		if (item->location == 2) // belt
		{
			item->row = item->column / 4;
			item->column %= 4;
		}
		//    if (location != 1) // not gear
		//        outStream << tr("item coordinates (%1, %2)\n").arg(row).arg(col);
		item->storage = bitReader.readNumber(3);
		//    if (!location)
		//        outStream << "storage " << storage << '\n';

		for (int i = 0; i < 3; ++i)
			item->itemType += static_cast<quint8>(bitReader.readNumber(8));
		bitReader.skip(8); // skip last space (byte 4 at 84-92)

		const ItemBase &itemBase = ItemDataBase::Items()->value(item->itemType);
		//    outStream << QString("item type '%1', name '%2', rlvl %3").arg(itemType, itemBase.name).arg(itemBase.rlvl);
		//    if (itemBase.classCode > -1)
		//        outStream << tr(" (%1 Only)").arg(Enums::ClassName::classes().at(itemBase.classCode));
		//    outStream << "\n\n";

		if (item->isExtended)
		{
			item->socketablesNumber = bitReader.readNumber(3);
			//outStream << QString("-----Extended item info-----\n\n%1 gems inserted\n").arg(gems);
			item->guid = bitReader.readNumber(32);
			//outStream << "GUID" << bitReader.readNumber(32) << '\n';
			item->ilvl = bitReader.readNumber(7);
			//outStream << "ilvl " << bitReader.readNumber(7) << '\n';
			item->quality = bitReader.readNumber(4);
			//outStream << "quality " << quality << '\n';
			if (bitReader.readBool())
				item->variableGraphicIndex = bitReader.readNumber(3) + 1;
			if (bitReader.readBool()) // class info
				bitReader.skip(11);
			switch (item->quality)
			{
			case Enums::ItemQuality::Normal:
				break;
			case Enums::ItemQuality::LowQuality: case Enums::ItemQuality::HighQuality:
				item->nonMagicType = bitReader.readNumber(3);
				break;
			case Enums::ItemQuality::Magic:
				bitReader.skip(22); // prefix & suffix
				break;
			case Enums::ItemQuality::Set: case Enums::ItemQuality::Unique:
				item->setOrUniqueId = bitReader.readNumber(12);
				break;
			case Enums::ItemQuality::Rare: case Enums::ItemQuality::Crafted:
				bitReader.skip(16); // first & second names
				for (int i = 0; i < 6; ++i)
					if (bitReader.readBool())
						bitReader.skip(11); // prefix or suffix (1-3)
				break;
			case Enums::ItemQuality::Honorific:
				bitReader.skip(16); // no idea what these bits mean
				break;
			default:
				ERROR_BOX_NO_PARENT(tr("Item '%1' of unknown quality '%2' found!").arg(itemBase.name).arg(item->quality));
				break;
			}

			//outStream << endl;
			if (item->isRW)
			{
				bitReader.skip(16);
				//outStream << "runeword code" << bitReader.readNumber(16) << '\n';
			}
			if (item->isPersonalized)
			{
				for (int i = 0; i < 16; ++i)
				{
					quint8 c = static_cast<quint8>(bitReader.readNumber(7));
					if (!c)
						break;
					item->inscribedName += c;
				}
				//outStream << "inscribed name: " << inscribedName << '\n';
			}
			bitReader.skip(); // tome of ID bit
			if (item->itemType == "ibk" || item->itemType == "tbk")
			{
				bitReader.skip(5); // some tome bits
			}
			if (itemBase.type == Enums::ItemType::Armor)
			{
				const ItemPropertyTxt &defenceProp = ItemDataBase::Properties()->value(Enums::ItemProperties::Defence);
				item->defense = bitReader.readNumber(defenceProp.bits) - defenceProp.add;
			}
			if (itemBase.type != Enums::ItemType::Misc)
			{
				const ItemPropertyTxt &maxDurabilityProp = ItemDataBase::Properties()->value(Enums::ItemProperties::DurabilityMax);
				item->maxDurability = bitReader.readNumber(maxDurabilityProp.bits) - maxDurabilityProp.add;
				if (item->maxDurability)
				{
					const ItemPropertyTxt &durabilityProp = ItemDataBase::Properties()->value(Enums::ItemProperties::Durability);
					item->currentDurability = bitReader.readNumber(durabilityProp.bits) - durabilityProp.add;
					if (item->maxDurability < item->currentDurability)
						item->maxDurability = item->currentDurability;
					//outStream << QString("durability: %1 of %2\n").arg(currentDurability).arg(maxDurability);
				}
			}
			item->quantity = itemBase.isStackable ? bitReader.readNumber(9) : -1;
			if (item->isSocketed)
				item->socketsNumber = bitReader.readNumber(4);

			bool hasSetLists[5] = {false};
			if (item->quality == Enums::ItemQuality::Set)
				for (int i = 0; i < 5; i++)
					hasSetLists[i] = bitReader.readBool(); // should always be false for MXL

			item->props = parseItemProperties(bitReader, &ok);
			if (!ok)
			{
				inputDataStream.device()->seek(itemStartOffset - 2); // set to JM - beginning of the item
				item->props[1].displayString = tr("Error parsing item properties, please report!");
				searchEndOffset = nextItemOffset + 1;
				continue;
			}
			else if (item->props.size())
				;//outStream << "\n-----Item properties-----\n\n" << itemProps;

			//        for (int i = 0; i < 5; ++i)
			//            if (hasSetLists[i])
			//                outStream << "set property list #" << i+1 << " should be here\n";

			if (item->isRW)
			{
				item->rwProps = parseItemProperties(bitReader, &ok);
				if (!ok)
				{
					inputDataStream.device()->seek(itemStartOffset - 2); // set to JM - beginning of the item
					item->rwProps[1].displayString = tr("Error parsing RW properties, please report!");
					searchEndOffset = nextItemOffset + 1;
					continue;
				}
				else if (item->rwProps.size())
					;//outStream << QString("-----RW properties-----\n\n%1").arg(parseItemProperties(bitReader));
			}

			// parse all socketables
			QList<QByteArray> runeTypes;
			for (int i = 0; i < item->socketablesNumber; ++i)
			{
				ItemInfo *socketableInfo = parseItem(inputDataStream, bytes);
				item->socketablesInfo += socketableInfo;
				//outStream << tr("Socket filler #%1:\n%2").arg(i + 1).arg(gemInfo);
				if (item->isRW && i >= item->socketablesNumber - 2) // get the last socket filler to obtain RW name (and previous one to prevent disambiguation)
				{
					runeTypes.prepend(socketableInfo->itemType == "jew" && i != item->socketablesNumber - 1 ? QByteArray() : socketableInfo->itemType);
				}
			}

			if (runeTypes.size())
			{
				RunewordKeyPair rwKey = qMakePair(runeTypes.at(0), runeTypes.size() > 1 ? runeTypes.at(1) : QByteArray());
				if (rwKey == qMakePair(QByteArray("r56"), QByteArray("r55"))) // maybe it's 'Eternal'?
					item->rwName = ItemDataBase::RW()->value(qMakePair(QByteArray("r51"), QByteArray("r52"))).name;
				else
				{
					QMultiHash<RunewordKeyPair, RunewordInfo>::const_iterator iter = ItemDataBase::RW()->find(rwKey);
					for (; iter != ItemDataBase::RW()->end() && iter.key() == rwKey; ++iter)
					{
						const RunewordInfo &rwInfo = iter.value();
						qDebug() << rwInfo.name << rwInfo.allowedItemTypes << itemBase.typeString;
						if (rwInfo.allowedItemTypes.contains(itemBase.typeString))
						{
							item->rwName = rwInfo.name;
							break;
						}
					}
					if (iter == ItemDataBase::RW()->end())
						item->rwName = tr("Unknown RW, please report!");
				}
			}
			else if (item->isRW)
				item->rwName = tr("Unknown RW (no socketables detected), please report!");
			//outStream << "\n\n-----End-----\n\n";
		}
		else
			item->quality = Enums::ItemQuality::Normal;
	} while (!ok && ++attempt < 5);
	return item;
}

QMap<int, ItemProperty> ItemParser::parseItemProperties(ReverseBitReader &bitReader, bool *ok)
{
	QMap<int, ItemProperty> props;
	while (bitReader.pos() != -1)
	{
		int id = bitReader.readNumber(Enums::CharacterStats::StatCodeLength);
		if (id == Enums::ItemProperties::End)
		{
			*ok = true;
			return props;
		}

		const ItemPropertyTxt &prop = ItemDataBase::Properties()->value(id);
		ItemProperty propToAdd;
		propToAdd.param = prop.saveParamBits ? bitReader.readNumber(prop.saveParamBits) : 0;
		propToAdd.value = bitReader.readNumber(prop.bits) - prop.add;
		if (id == 17) // max edamage%
		{
			//            qint16 minEnhDamage = bitReader.readNumber(prop.bits) - prop.add;
			bitReader.skip(prop.bits);
			propToAdd.displayString = tr("+%1% Enhanced Damage").arg(propToAdd.value);
			// minEnhDamage != propToAdd.value only for superior items that have been MO'd with ED
			//            if (minEnhDamage != propToAdd.value)
			//                propToAdd.displayString += " " + tr("[min ed %1 != max ed %2!!! please report]").arg(minEnhDamage).arg(propToAdd.value);
		}

		// elemental damage
		bool hasLength = false, hasMinElementalDamage = false;
		if (id == 48 || id == 50 || id == 52 || id == 54 || id == 57) // min elemental damage
		{
			if (id == 54 || id == 57) // cold or poison
				hasLength = true; // length is present only when min damage is specified
			else if (id == 52) // +%d magic damage
			{
				QString desc = prop.descPositive;
				propToAdd.displayString = desc.replace("%d", "%1").arg(propToAdd.value);
			}
			props[id++] = propToAdd;
			hasMinElementalDamage = true;
		}
		if (hasMinElementalDamage)
		{
			// get max elemental damage
			const ItemPropertyTxt &maxElementalDamageProp = ItemDataBase::Properties()->value(id);
			propToAdd.value = bitReader.readNumber(maxElementalDamageProp.bits) - maxElementalDamageProp.add;

			if (id == 53) // +%d magic damage
			{
				QString desc = maxElementalDamageProp.descPositive;
				propToAdd.displayString = desc.replace("%d", "%1").arg(propToAdd.value);
			}
			props[id] = propToAdd;

			if (hasLength) // cold or poison length
			{
				const ItemPropertyTxt &lengthProp = ItemDataBase::Properties()->value(++id);
				qint16 length = bitReader.readNumber(lengthProp.bits) - lengthProp.add;
				//                propToAdd.displayString = QString(" with length of %1 frames (%2 second(s))").arg(length).arg(static_cast<double>(length) / 25.0, 1);

				//				propToAdd.value = length;
				//				props[id] = propToAdd;

				if (id == 59) // poison length
					props[id - 1].value = props[id - 2].value = qRound(props[id - 2].value * length / 256.0); // set correct min/max poison damage
			}

			continue;
		}

		if (id == 83)
			propToAdd.displayString = tr("+%1 to %2 Skill Levels").arg(propToAdd.value).arg(Enums::ClassName::classes().at(propToAdd.param));
		else if (id == 97 || id == 107)
		{
			const SkillInfo &skill = ItemDataBase::Skills()->at(propToAdd.param);
			propToAdd.displayString = tr("+%1 to %2").arg(propToAdd.value).arg(skill.name);
			if (id == 107)
				propToAdd.displayString = " " + tr("(%1 Only)", "class-specific skill").arg(skill.classCode > -1 ? Enums::ClassName::classes().at(skill.classCode) : "FAIL");
		}
		else if (id == 204)
		{
			propToAdd.displayString = tr("Level %1 %2 (%3/%4 Charges)").arg(propToAdd.param & 63).arg(ItemDataBase::Skills()->value(propToAdd.param >> 6).name)
				.arg(propToAdd.value & 255).arg(propToAdd.value >> 8);
		}
		//        else if (id == 219)
		//            propToAdd.displayString = "trophy'd charm or blessed craft";
		else if (QString(prop.descPositive).startsWith('%')) // ctc
		{
			QString desc = prop.descPositive;
			for (int i = 0, k = 1; k <= 3 && i < desc.length(); ++i)
				if (desc.at(i) == '%' && desc.at(i + 1).isLetter())
					desc[++i] = QString::number(k++).at(0);
			propToAdd.displayString = desc.replace("%%", "%").arg(propToAdd.value).arg(propToAdd.param & 63).arg(ItemDataBase::Skills()->value(propToAdd.param >> 6).name);
		}
		else if (ItemDataBase::MysticOrbs()->contains(id))
		{
			propToAdd.displayString = QString("%1 x '%2'").arg(propToAdd.value).arg(ItemDataBase::Items()->value(ItemDataBase::MysticOrbs()->value(id).itemCode).name);
		}

		props[id] = propToAdd;
	}

	*ok = false;
	return QMap<int, ItemProperty>();
}

void ItemParser::writeItems(const ItemsList &items, QDataStream &ds)
{
	foreach (ItemInfo *item, items)
	{
		ds.writeRawData(itemHeader.constData(), itemHeader.size()); // do not write '\0'

		QByteArray itemBytes;
		for (int i = 0; i < item->bitString.length(); i += 8)
			itemBytes.prepend(item->bitString.mid(i, 8).toShort(0, 2));
		ds.writeRawData(itemBytes.constData(), itemBytes.size());

		writeItems(item->socketablesInfo, ds);
	}
}

ItemInfo *ItemParser::loadItemFromFile(const QString &filePath)
{
	ItemInfo *item = 0;
	QFile itemFile(filePath);
	if (itemFile.open(QIODevice::ReadOnly))
	{
		QByteArray itemBytes = itemFile.readAll();
		itemFile.close();

		QDataStream ds(itemBytes);
		ds.setByteOrder(QDataStream::LittleEndian);

		item = parseItem(ds, itemBytes);
		item->hasChanged = true;
		item->row = item->column = -1;
	}
	else
		ERROR_BOX_NO_PARENT(tr("Error opening file '%1'\nReason: %2").arg(filePath).arg(itemFile.errorString()));
	return item;
}

ItemsList ItemParser::itemsLocatedAt(int storage, bool location /*= Enums::ItemLocation::Stored*/)
{
	ItemsList items, *characterItems = ItemDataBase::currentCharacterItems;
	for (int i = 0; i < characterItems->size(); ++i)
	{
		ItemInfo *item = characterItems->at(i);
		if (item->location == location && item->storage == storage)
			items += item;
	}
	return items;
}

bool ItemParser::storeItemIn(Enums::ItemStorage::ItemStorageEnum storage, quint8 rows, quint8 cols, ItemInfo *item)
{
	ItemsList items = itemsLocatedAt(storage);
	for (quint8 i = 0; i < rows; ++i)
		for (quint8 j = 0; j < cols; ++j)
			if (canStoreItemAt(i, j, item->itemType, items))
			{
				item->storage = storage;
				item->row = i;
				item->column = j;
				return true;
			}
			return false;
}

bool ItemParser::canStoreItemAt(quint8 row, quint8 col, const QByteArray &itemType, const ItemsList &items)
{
	const ItemBase &storeItemBase = ItemDataBase::Items()->value(itemType);
	QRect storeItemRect(row, col, storeItemBase.width - 1, storeItemBase.height - 1);

	foreach (ItemInfo *item, items)
	{
		const ItemBase &itemBase = ItemDataBase::Items()->value(item->itemType);
		if (!storeItemRect.intersects(QRect(item->row, item->column, itemBase.width - 1, itemBase.height - 1)))
			return true;
	}
	return false;
}
