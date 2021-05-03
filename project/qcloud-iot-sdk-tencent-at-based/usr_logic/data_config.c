/*-----------------data config start  -------------------*/ 

#define TOTAL_PROPERTY_COUNT 8

static sDataPoint    sg_DataTemplate[TOTAL_PROPERTY_COUNT];

typedef struct _ProductDataDefine {
    TYPE_DEF_TEMPLATE_BOOL m_power_switch;
    TYPE_DEF_TEMPLATE_FLOAT m_current;
    TYPE_DEF_TEMPLATE_FLOAT m_voltage;
    TYPE_DEF_TEMPLATE_FLOAT m_power_factor;
    TYPE_DEF_TEMPLATE_FLOAT m_active_power;
    TYPE_DEF_TEMPLATE_FLOAT m_apparent_power;
    TYPE_DEF_TEMPLATE_FLOAT m_total_kwh;
    TYPE_DEF_TEMPLATE_INT m_count_down;
} ProductDataDefine;

static   ProductDataDefine     sg_ProductData;

static void _init_data_template(void)
{
    sg_ProductData.m_power_switch = 0;
    sg_DataTemplate[0].data_property.data = &sg_ProductData.m_power_switch;
    sg_DataTemplate[0].data_property.key  = "power_switch";
    sg_DataTemplate[0].data_property.type = TYPE_TEMPLATE_BOOL;

    sg_ProductData.m_current = 0;
    sg_DataTemplate[1].data_property.data = &sg_ProductData.m_current;
    sg_DataTemplate[1].data_property.key  = "current";
    sg_DataTemplate[1].data_property.type = TYPE_TEMPLATE_FLOAT;

    sg_ProductData.m_voltage = 0;
    sg_DataTemplate[2].data_property.data = &sg_ProductData.m_voltage;
    sg_DataTemplate[2].data_property.key  = "voltage";
    sg_DataTemplate[2].data_property.type = TYPE_TEMPLATE_FLOAT;

    sg_ProductData.m_power_factor = 0;
    sg_DataTemplate[3].data_property.data = &sg_ProductData.m_power_factor;
    sg_DataTemplate[3].data_property.key  = "power_factor";
    sg_DataTemplate[3].data_property.type = TYPE_TEMPLATE_FLOAT;

    sg_ProductData.m_active_power = 0;
    sg_DataTemplate[4].data_property.data = &sg_ProductData.m_active_power;
    sg_DataTemplate[4].data_property.key  = "active_power";
    sg_DataTemplate[4].data_property.type = TYPE_TEMPLATE_FLOAT;

    sg_ProductData.m_apparent_power = 0;
    sg_DataTemplate[5].data_property.data = &sg_ProductData.m_apparent_power;
    sg_DataTemplate[5].data_property.key  = "apparent_power";
    sg_DataTemplate[5].data_property.type = TYPE_TEMPLATE_FLOAT;

    sg_ProductData.m_total_kwh = 0;
    sg_DataTemplate[6].data_property.data = &sg_ProductData.m_total_kwh;
    sg_DataTemplate[6].data_property.key  = "total_kwh";
    sg_DataTemplate[6].data_property.type = TYPE_TEMPLATE_FLOAT;

    sg_ProductData.m_count_down = 0;
    sg_DataTemplate[7].data_property.data = &sg_ProductData.m_count_down;
    sg_DataTemplate[7].data_property.key  = "count_down";
    sg_DataTemplate[7].data_property.type = TYPE_TEMPLATE_INT;

};
