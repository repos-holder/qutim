#include "dataforms.h"
#include "dynamicpropertydata_p.h"
#include <QPointer>

Q_DECLARE_METATYPE(QList<QIcon>);
Q_DECLARE_METATYPE(QList<QPixmap>);
Q_DECLARE_METATYPE(QList<QImage>);
Q_DECLARE_METATYPE(QValidator*);

namespace qutim_sdk_0_3
{
	class DataItemPrivate : public DynamicPropertyData
	{
	public:
		DataItemPrivate()  :
				maxCount(1) {}
		QString name;
		LocalizedString title;
		QVariant data;
		QList<DataItem> subitems;
		int maxCount;
		DataItem defaultSubitem;

		QVariant getName() const { return QVariant::fromValue(name); }
		void setName(const QVariant &val) { name = val.value<QString>(); }
		QVariant getTitle() const { return QVariant::fromValue(title); }
		void setTitle(const QVariant &val) { title = val.value<LocalizedString>(); }
		QVariant getData() const { return data; }
		void setData(const QVariant &val) { data = val; }
		QVariant getMaxCount() const { return maxCount; }
		void setMaxCount(const QVariant &val) { maxCount = val.toInt(); }
		QVariant getDefaultSubitem() const { return QVariant::fromValue(defaultSubitem); }
		void setDefaultSubitem(const QVariant &val) { defaultSubitem = val.value<DataItem>(); }
	};

	namespace CompiledProperty
	{
		static QList<QByteArray> names = QList<QByteArray>()
										 << "name"
										 << "title"
										 << "data"
										 << "maxCount"
										 << "defaultSubitem";
		static QList<Getter> getters   = QList<Getter>()
										 << static_cast<Getter>(&DataItemPrivate::getName)
										 << static_cast<Getter>(&DataItemPrivate::getTitle)
										 << static_cast<Getter>(&DataItemPrivate::getData)
										 << static_cast<Getter>(&DataItemPrivate::getMaxCount)
										 << static_cast<Getter>(&DataItemPrivate::getDefaultSubitem);
		static QList<Setter> setters   = QList<Setter>()
										 << static_cast<Setter>(&DataItemPrivate::setName)
										 << static_cast<Setter>(&DataItemPrivate::setTitle)
										 << static_cast<Setter>(&DataItemPrivate::setData)
										 << static_cast<Setter>(&DataItemPrivate::setMaxCount)
										 << static_cast<Setter>(&DataItemPrivate::setDefaultSubitem);
	}

	static inline void ensure_data(QSharedDataPointer<DataItemPrivate> &d)
	{
		if (!d)
			d = new DataItemPrivate;
	}

	DataItem::DataItem()
	{
	}

	DataItem::DataItem(const QString &name, const LocalizedString &title, const QVariant &data) :
			d(new DataItemPrivate)
	{
		d->name = name;
		d->title = title;
		d->data = data;
	}

	DataItem::DataItem(const LocalizedString &title, const QVariant &data) :
			d(new DataItemPrivate)
	{
		d->title = title;
		d->data = data;
	}

	DataItem::DataItem(const DataItem &other) :
			d(other.d)
	{
	}

	DataItem::~DataItem()
	{
	}

	DataItem &DataItem::operator=(const DataItem &other)
	{
		d = other.d;
		return *this;
	}

	QString DataItem::name() const
	{
		return d->name;
	}

	void DataItem::setName(const QString &itemName)
	{
		ensure_data(d);
		d->name = itemName;
	}

	LocalizedString DataItem::title() const
	{
		return d->title;
	}

	void DataItem::setTitle(const LocalizedString &itemTitle)
	{
		d->title = itemTitle;
	}

	QVariant DataItem::data() const
	{
		if (!d) return QVariant();
		return d->data;
	}

	void DataItem::setData(const QVariant &itemData)
	{
		ensure_data(d);
		d->data = itemData;
	}

	bool DataItem::isNull() const
	{
		return !d.constData();
	}

	QList<DataItem> DataItem::subitems() const
	{
		return d->subitems;
	}

	void DataItem::setSubitems(const QList<DataItem> &newSubitems)
	{
		ensure_data(d);
		d->subitems = newSubitems;
	}

	DataItem DataItem::subitem(const QString &name, bool recursive) const
	{
		if (!d)
			return DataItem();
		foreach (const DataItem &item, d->subitems) {
			if (item.name() == name)
				return item;
			if (recursive) {
				DataItem res = item.subitem(name);
				if (!res.isNull())
					return res;
			}
		}
		return DataItem();
	}

	void DataItem::addSubitem(const DataItem &subitem)
	{
		ensure_data(d);
		d->subitems << subitem;
	}

	bool DataItem::hasSubitems() const
	{
		return !d->subitems.isEmpty();
	}

	void DataItem::allowModifySubitems(const DataItem &defaultSubitem, int maxSubitemsCount)
	{
		ensure_data(d);
		d->defaultSubitem = defaultSubitem;
		d->maxCount = maxSubitemsCount;
	}

	bool DataItem::isAllowedModifySubitems() const
	{
		return d->maxCount != 1 && !isReadOnly();
	}

	int DataItem::maxSubitemsCount() const
	{
		return d->maxCount;
	}

	DataItem DataItem::defaultSubitem() const
	{
		return d->defaultSubitem;
	}

	bool DataItem::isReadOnly() const
	{
		return property("readOnly", false);
	}

	void DataItem::setReadOnly(bool readOnly)
	{
		setProperty("readOnly", readOnly);
	}

	DataItem &DataItem::operator<<(const DataItem &subitem)
	{
		ensure_data(d);
		d->subitems << subitem;
		return *this;
	}

	QVariant DataItem::property(const char *name, const QVariant &def) const
	{
		return d->property(name, def, CompiledProperty::names, CompiledProperty::getters);
	}

	void DataItem::setProperty(const char *name, const QVariant &value)
	{
		ensure_data(d);
		d->setProperty(name, value, CompiledProperty::names, CompiledProperty::setters);
	}

	ReadOnlyDataItem::ReadOnlyDataItem(const LocalizedString &title, const QStringList &data) :
		DataItem(title, data)
	{
	}

	ReadOnlyDataItem::ReadOnlyDataItem(const LocalizedString &title, const LocalizedStringList &data) :
		DataItem(title, qVariantFromValue(data))
	{
	}

	ReadOnlyDataItem::ReadOnlyDataItem(const LocalizedString &title, bool data) :
		DataItem(title, data)
	{
	}

	ReadOnlyDataItem::ReadOnlyDataItem(const LocalizedString &title, const QDate &data) :
		DataItem(title, data)
	{
	}

	ReadOnlyDataItem::ReadOnlyDataItem(const LocalizedString &title, const QDateTime &data) :
		DataItem(title, data)
	{
	}

	ReadOnlyDataItem::ReadOnlyDataItem(const LocalizedString &title, int data) :
		DataItem(title, data)
	{
	}

	ReadOnlyDataItem::ReadOnlyDataItem(const LocalizedString &title, uint data) :
		DataItem(title, data)
	{
	}

	ReadOnlyDataItem::ReadOnlyDataItem(const LocalizedString &title, double data) :
		DataItem(title, data)
	{
	}

	ReadOnlyDataItem::ReadOnlyDataItem(const LocalizedString &title, const QIcon &data) :
		DataItem(title, data)
	{
	}

	ReadOnlyDataItem::ReadOnlyDataItem(const LocalizedString &title, const QImage &data) :
		DataItem(title, data)
	{
	}

	ReadOnlyDataItem::ReadOnlyDataItem(const LocalizedString &title, const QPixmap &data) :
		DataItem(title, data)
	{
	}

	ReadOnlyDataItem::ReadOnlyDataItem(const LocalizedString &title, const QString &data) :
		DataItem(title, data)
	{
	}

	ReadOnlyDataItem::ReadOnlyDataItem(const LocalizedString &title, const LocalizedString &data) :
		DataItem(title, qVariantFromValue(data))
	{
	}

	StringListDataItem::StringListDataItem(const QString &name, const LocalizedString &title,
										   const QStringList &data, int maxStringsCount) :
		DataItem(name, title, data)
	{
		setProperty("maxStringsCount", maxStringsCount);
	}

	StringListDataItem::StringListDataItem(const QString &name, const LocalizedString &title,
										   const LocalizedStringList &data, int maxStringsCount) :
		DataItem(name, title, qVariantFromValue(data))
	{
		setProperty("maxStringsCount", maxStringsCount);
	}

	IntDataItem::IntDataItem(const QString &name, const LocalizedString &title, int data,
							 int minValue, int maxValue) :
		DataItem(name, title, data)
	{
		setProperty("minValue", minValue);
		setProperty("maxValue", maxValue);
	}

	DoubleDataItem::DoubleDataItem(const QString &name, const LocalizedString &title, double data,
								   double minValue, double maxValue) :
		DataItem(name, title, data)
	{
		setProperty("minValue", minValue);
		setProperty("maxValue", maxValue);
	}

	IconDataItem::IconDataItem(const QString &name, const LocalizedString &title, const QIcon &data,
							   const QSize &imageSize, const QList<QIcon> &alternatives) :
		DataItem(name, title, data)
	{
		setProperty("imageSize", imageSize);
		setProperty("alternatives", qVariantFromValue(alternatives));
	}

	ImageDataItem::ImageDataItem(const QString &name, const LocalizedString &title, const QImage &data,
								 const QSize &imageSize, const QList<QImage> &alternatives) :
		DataItem(name, title, data)
	{
		setProperty("imageSize", imageSize);
		setProperty("alternatives", qVariantFromValue(alternatives));
	}

	PixmapDataItem::PixmapDataItem(const QString &name, const LocalizedString &title, const QPixmap &data,
								   const QSize &imageSize, const QList<QPixmap> &alternatives) :
		DataItem(name, title, data)
	{
		setProperty("imageSize", imageSize);
		setProperty("alternatives", qVariantFromValue(alternatives));
	}

	StringChooserDataItem::StringChooserDataItem(const QString &name, const LocalizedString &title,
												 const QStringList &alternatives, const QString &data,
												 bool editable, QValidator *validator) :
		DataItem(name, title, data)
	{
		setProperty("alternatives", qVariantFromValue(alternatives));
		setProperty("editable", editable);
		if (validator)
			setProperty("validator", qVariantFromValue(validator));
	}

	StringChooserDataItem::StringChooserDataItem(const QString &name, const LocalizedString &title,
												 const QStringList &alternatives, const QString &data,
												 bool editable, QRegExp validator) :
		DataItem(name, title, data)
	{
		setProperty("alternatives", qVariantFromValue(alternatives));
		setProperty("editable", editable);
		if (!validator.isEmpty())
			setProperty("validator", qVariantFromValue(validator));
	}

	StringChooserDataItem::StringChooserDataItem(const QString &name, const LocalizedString &title,
												 const LocalizedStringList &alternatives, const LocalizedString &data,
												 bool editable, QValidator *validator) :
		DataItem(name, title, qVariantFromValue(data))
	{
		setProperty("alternatives", qVariantFromValue(alternatives));
		setProperty("editable", editable);
		if (validator)
			setProperty("validator", qVariantFromValue(validator));
	}

	StringChooserDataItem::StringChooserDataItem(const QString &name, const LocalizedString &title,
												 const LocalizedStringList &alternatives, const LocalizedString &data,
												 bool editable, QRegExp validator) :
		DataItem(name, title, qVariantFromValue(data))
	{
		setProperty("alternatives", qVariantFromValue(alternatives));
		setProperty("editable", editable);
		if (!validator.isEmpty())
			setProperty("validator", qVariantFromValue(validator));
	}

	MultiLineStringDataItem::MultiLineStringDataItem(const QString &name, const LocalizedString &title,
													 const QString &data) :
		DataItem(name, title, data)
	{
		setProperty("multiline", true);
	}

	MultiLineStringDataItem::MultiLineStringDataItem(const QString &name, const LocalizedString &title,
													 const LocalizedString &data) :
		DataItem(name, title, qVariantFromValue(data))
	{
		setProperty("multiline", true);
	}

	StringDataItem::StringDataItem(const QString &name, const LocalizedString &title,
								   const QString &data, QValidator *validator, bool password) :
		DataItem(name, title, data)
	{
		if (validator)
			setProperty("validator", qVariantFromValue(validator));
		setProperty("password", password);
	}

	StringDataItem::StringDataItem(const QString &name, const LocalizedString &title,
								   const QString &data, QRegExp validator, bool password) :
		DataItem(name, title, data)
	{
		if (!validator.isEmpty())
			setProperty("validator", qVariantFromValue(validator));
		setProperty("password", password);
	}

	StringDataItem::StringDataItem(const QString &name, const LocalizedString &title,
								   const LocalizedString &data, QValidator *validator, bool password) :
		DataItem(name, title, qVariantFromValue(data))
	{
		if (validator)
			setProperty("validator", qVariantFromValue(validator));
		setProperty("password", true);
	}

	StringDataItem::StringDataItem(const QString &name, const LocalizedString &title,
								   const LocalizedString &data, QRegExp validator, bool password) :
		DataItem(name, title, qVariantFromValue(data))
	{
		if (!validator.isEmpty())
			setProperty("validator", qVariantFromValue(validator));
		setProperty("password", true);
	}

	void AbstractDataWidget::virtual_hook(int id, void *data)
	{
		Q_UNUSED(id);
		Q_UNUSED(data);
	}

	QWidget *AbstractDataForm::get(const DataItem &item, StandardButtons standartButtons,
							const AbstractDataForm::Buttons &buttons)
	{
		DataFormsBackend *b = DataFormsBackend::instance();
		if (b)
			return b->get(item, standartButtons, buttons);
		else
			return 0;
	}

	void AbstractDataForm::accept()
	{
		emit accepted();
	}

	void AbstractDataForm::reject()
	{
		emit rejected();
	}

	DataFormsBackend *DataFormsBackend::instance()
	{
		static QPointer<DataFormsBackend> self;
		if(self.isNull() && isCoreInited())
			self = qobject_cast<DataFormsBackend *>(getService("DataFormsBackend"));
		return self;
	}

}
