#include <stdio.h>

#include <libadwaita-1/adwaita.h>

#include "service.h"
#include "util.h"
#include "gui_state.h"
#include "gui.h"

typedef struct PizzaOrderGuiLine {
    size_t id;
    GtkBox* box;
    GtkComboBox* combo;
    GtkSpinButton* spin;
} PizzaOrderGuiLine;

typedef struct PizzaDetailedOrder {
    size_t id;
    GtkListBoxRow* row;
} PizzaDetailedOrder;

void addPizzaOrderLine();
const char* getEntryText(GtkEntry* entry);
const char* getPizzaSelectionText(GtkWidget* comboBox);
void renderUI();

GtkBuilder* builder = NULL;

GtkWidget* window = NULL;
GtkWidget* lastName = NULL;
GtkWidget* street = NULL;
GtkWidget* building = NULL;
GtkWidget* apartment = NULL;
GtkWidget* addPizzas = NULL;
GtkWidget* phone = NULL;
GtkWidget* pizzaSelectBox = NULL;
GtkWidget* statusLabel = NULL;
GtkWidget* orderButton = NULL;
GtkWidget* ordersListWidget = NULL;
GtkWidget* sortButton = NULL;
GtkWidget* searchButton = NULL;

PizzaOrderGuiLine pizzaOrderLines[20];
int pizzaOrderLinesCount = 0;

PizzaDetailedOrder detailedOrders[20];
int detailedOrdersCount = 0;

void pizzaOrderLinesAppend(PizzaOrderGuiLine line)
{
    if (pizzaOrderLinesCount < 20) {
        pizzaOrderLines[pizzaOrderLinesCount] = line;
        pizzaOrderLinesCount++;
    }
}

void pizzaOrderLinesRemove(int index)
{
    if (index < pizzaOrderLinesCount) {
        pizzaOrderLinesCount--;
        for (int i = index; i < pizzaOrderLinesCount; i++) {
            pizzaOrderLines[i] = pizzaOrderLines[i + 1];
        }
    }
}

void detailedOrdersRemove(int index)
{
    if (index < detailedOrdersCount) {
        detailedOrdersCount--;
        for (int i = index; i < detailedOrdersCount; i++) {
            detailedOrders[i] = detailedOrders[i + 1];
        }
    }
}

void deleteAllPizzaOrderLines()
{
    for (int i = 0; i < pizzaOrderLinesCount; i++) {
        gtk_box_remove(GTK_BOX(pizzaSelectBox), GTK_WIDGET(pizzaOrderLines[i].box));
    }
    pizzaOrderLinesCount = 0;
}

void deleteAllDetailedOrders()
{
    for (int i = 0; i < detailedOrdersCount; i++) {
        if (gtk_widget_is_visible(GTK_WIDGET(detailedOrders[i].row))) {
            gtk_list_box_remove(GTK_LIST_BOX(ordersListWidget), GTK_WIDGET(detailedOrders[i].row));
        }
    }
    detailedOrdersCount = 0;
}

static void onLastNameEditCallback(GtkWidget* widget)
{
    stateChangeMakingOrderLastName((char*)getEntryText(GTK_ENTRY(lastName)));
    renderUI();
}

static void onStreetEditCallback(GtkWidget* widget)
{
    stateChangeMakingOrderStreet((char*)getEntryText(GTK_ENTRY(street)));
    renderUI();
}

static void onBuildingEditCallback(GtkWidget* widget)
{
    char* buildingText = (char*)getEntryText(GTK_ENTRY(building));
    if (validateNumber(buildingText)) {
        stateChangeMakingOrderBuilding(atoi(buildingText));
    } else {
        stateChangeMakingOrderBuilding(0);
    }
    renderUI();
}

static void onApartmentEditCallback(GtkWidget* widget)
{
    char* apartmentText = (char*)getEntryText(GTK_ENTRY(apartment));
    if (validateNumber(apartmentText)) {
        stateChangeMakingOrderApartment(atoi(apartmentText));
    } else {
        stateChangeMakingOrderApartment(0);
    }
    renderUI();
}

static void onPhoneEditCallback(GtkWidget* widget)
{
    stateChangeMakingOrderPhone((char*)getEntryText(GTK_ENTRY(phone)));
    renderUI();
}


static void onComboBoxEditCallback(GtkWidget *widget, gpointer data)
{
    int pizzasId = GPOINTER_TO_INT(data);

    int order = 0;
    for (int i = 0; i < pizzaOrderLinesCount; i++) {
        if (pizzaOrderLines[i].id == pizzasId) {
            stateChangeMakingOrderOrderLineIndex(
                i,
                gtk_combo_box_get_active(pizzaOrderLines[i].combo)
            );
            break;
        }
        order++;
    }

    renderUI();
}
static void onSpinButtonEditCallback(GtkWidget *widget, gpointer data)
{
    size_t pizzasId = GPOINTER_TO_INT(data);

    for (int i = 0; i < pizzaOrderLinesCount; i++) {
        if (pizzaOrderLines[i].id == pizzasId) {
            stateChangeMakingOrderOrderLineCount(
                i,
                gtk_spin_button_get_value_as_int(pizzaOrderLines[i].spin)
            );
            break;
        }
    }

    renderUI();
}

static void onOrderCallback(GtkWidget *widget)
{
    addOrder(getMakingOrder());
    stateResetMakingOrder();
}

static void onAddPizzasCallback(GtkWidget* widget)
{
    stateAddMakingOrderOrderLine(0, 1);
    addPizzaOrderLine(0, 0);
    
    renderUI();
}

static void onDeletePizzasCallback(GtkWidget* widget, gpointer data)
{
    int pizzasId = GPOINTER_TO_INT(data);

    GtkBox* pizzasLine;
    for (int i = 0; i < pizzaOrderLinesCount; i++) {
        if (pizzaOrderLines[i].id == pizzasId) {
            pizzasLine = pizzaOrderLines[i].box;
            pizzaOrderLinesRemove(i);
            stateRemoveMakingOrderOrderLine(i);
            break;
        }
    }

    gtk_box_remove(GTK_BOX(pizzaSelectBox), GTK_WIDGET(pizzasLine));

    renderUI();
}

const char* getPizzaSelectionText(GtkWidget* comboBox)
{
    return gtk_entry_buffer_get_text(
        GTK_ENTRY_BUFFER(
            gtk_entry_get_buffer(
                GTK_ENTRY(
                    gtk_combo_box_get_child(GTK_COMBO_BOX(comboBox))
                )
            )
        )
    );
}

const char* getEntryText(GtkEntry* entry)
{
    return gtk_entry_buffer_get_text(
        GTK_ENTRY_BUFFER(
            gtk_entry_get_buffer(entry)
        )
    );
}

GtkListStore* arrayToListStore(char** array, int arrayLen)
{
    GtkTreeIter iter;
    GtkListStore* treeModel = gtk_list_store_new(1, G_TYPE_STRING);

    for (int i = 0; i < arrayLen; i++) {
        gtk_list_store_append(GTK_LIST_STORE(treeModel), &iter);
        gtk_list_store_set(GTK_LIST_STORE(treeModel), &iter, 0, array[i], -1);
    }

    return treeModel;
}

void addPizzaOrderLine(int selectedPizza, int pizzaCount)
{
    if (pizzaOrderLinesCount >= 20) {
        return;
    }

    int stockSize = 0;
    StockLine* stock = getStock(&stockSize);
    char** pizzas = malloc(sizeof(char*) * stockSize);
    for (int i = 0; i < stockSize; i++) {
        pizzas[i] = malloc(64);
        strcpy(pizzas[i], stock[i].pizzaName);
    }

    GtkWidget* comboBox = gtk_combo_box_new_with_model_and_entry(
        GTK_TREE_MODEL(
            arrayToListStore(pizzas, stockSize)
        )
    );
    gtk_widget_set_margin_end(comboBox, 4);
    gtk_editable_set_editable(
        GTK_EDITABLE(gtk_combo_box_get_child(GTK_COMBO_BOX(comboBox))),
        FALSE
    );
    gtk_widget_set_hexpand(comboBox, TRUE);
    gtk_combo_box_set_button_sensitivity(GTK_COMBO_BOX(comboBox), GTK_SENSITIVITY_ON);
    gtk_combo_box_set_entry_text_column(GTK_COMBO_BOX(comboBox), 0);
    gtk_combo_box_set_id_column(GTK_COMBO_BOX(comboBox), 0);

    GtkAdjustment* adjustment = gtk_adjustment_new(1, 1, 100, 1, 10, 1);
    
    GtkWidget* spinButton = gtk_spin_button_new(GTK_ADJUSTMENT(adjustment), 1, 0);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spinButton), pizzaCount);
    
    GtkWidget* deleteButton = gtk_button_new_with_label("Удалить");
    gtk_widget_set_margin_start(deleteButton, 4);

    GtkBox* box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3));
    gtk_widget_set_margin_bottom(GTK_WIDGET(box), 4);
    gtk_box_append(box, comboBox);
    gtk_box_append(box, spinButton);
    gtk_box_append(box, deleteButton);

    static int pizzasId = 0;

    char pizzaId[32];

    sprintf(pizzaId, "comboBox%d", pizzasId);

    gtk_widget_set_name(comboBox, pizzaId);

    pizzaOrderLinesAppend((PizzaOrderGuiLine){
        .id = pizzasId,
        .box = box,
        .combo = GTK_COMBO_BOX(comboBox),
        .spin = GTK_SPIN_BUTTON(spinButton),
    });

    g_signal_connect(
        deleteButton, 
        "clicked", 
        G_CALLBACK(onDeletePizzasCallback), 
        (gpointer)(size_t)pizzasId
    );

    g_signal_connect(
        comboBox, 
        "changed", 
        G_CALLBACK(onComboBoxEditCallback), 
        (gpointer)(size_t)pizzasId
    );

    g_signal_connect(
        spinButton, 
        "changed", 
        G_CALLBACK(onSpinButtonEditCallback), 
        (gpointer)(size_t)pizzasId
    );

    pizzasId++;
    gtk_box_append(GTK_BOX(pizzaSelectBox), GTK_WIDGET(box));
    gtk_combo_box_set_active(GTK_COMBO_BOX(comboBox), selectedPizza);
}

static void onOrderDeleteClicked(GtkWidget* widget, gpointer data)
{
    int id = GPOINTER_TO_INT(data);
    deleteOrder(id);
    renderUI();
}

static void onSearchDialogLastnameClicked(GtkWidget* widget, gpointer data)
{
    const char* text = getEntryText((GtkEntry*)data);
    stateFilterByLastName((char*)text);
    renderUI();
}

static void onSearchDialogCouriersLastnameClicked(GtkWidget* widget, gpointer data)
{
    const char* text = getEntryText((GtkEntry*)data);
    stateFilterByCourierLastName((char*)text);
    renderUI();
}

static void onSearchDialogStatusClicked(GtkWidget* widget, gpointer data)
{
    OrderStatus status = -1;
    const char* text = getEntryText((GtkEntry*)data);
    if (strstr("доставляется", text)) {
        status = Delivery;
    } else if (strstr("доставлен", text)) {
        status = Delivered;
    } else {
        return;
    }

    stateFilterByStatus(status);
    renderUI();
}

static void onSearchDialogPizzaClicked(GtkWidget* widget, gpointer data)
{
    const char* text = getEntryText((GtkEntry*)data);
    stateFilterByPizza((char*)text);
    renderUI();
}

static void onSearchClicked(GtkWidget* widget, gpointer data)
{
    GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL;
    GtkWidget *contentArea;
    GtkDialog *dialog = GTK_DIALOG(gtk_dialog_new_with_buttons(
        "Поиск заказов",
        GTK_WINDOW(window),
        flags,
        "OK",
        GTK_RESPONSE_NONE,
        NULL
    ));
    contentArea = gtk_dialog_get_content_area(dialog);

    GtkBox *mainBox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 4));

    GtkBox *searchByLastNameBox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4));
    GtkLabel *lastnameLabel = GTK_LABEL(gtk_label_new("Поиск по фамилии заказчика: "));
    gtk_widget_set_margin_start(GTK_WIDGET(lastnameLabel), 4);
    gtk_widget_set_margin_end(GTK_WIDGET(lastnameLabel), 4);
    gtk_widget_set_margin_top(GTK_WIDGET(lastnameLabel), 4);
    gtk_widget_set_margin_bottom(GTK_WIDGET(lastnameLabel), 4);

    GtkEntry *lastnameEntry = GTK_ENTRY(gtk_entry_new());
    gtk_widget_set_hexpand(GTK_WIDGET(lastnameEntry), TRUE);
    gtk_widget_set_margin_start(GTK_WIDGET(lastnameEntry), 4);
    gtk_widget_set_margin_end(GTK_WIDGET(lastnameEntry), 4);
    gtk_widget_set_margin_top(GTK_WIDGET(lastnameEntry), 4);
    gtk_widget_set_margin_bottom(GTK_WIDGET(lastnameEntry), 4);

    GtkButton *lastnameButton = GTK_BUTTON(gtk_button_new_with_label("Поиск"));
    gtk_widget_set_hexpand(GTK_WIDGET(lastnameButton), TRUE);
    gtk_widget_set_margin_start(GTK_WIDGET(lastnameButton), 4);
    gtk_widget_set_margin_end(GTK_WIDGET(lastnameButton), 4);
    gtk_widget_set_margin_top(GTK_WIDGET(lastnameButton), 4);
    gtk_widget_set_margin_bottom(GTK_WIDGET(lastnameButton), 4);

    g_signal_connect(
        lastnameButton, 
        "clicked", 
        G_CALLBACK(onSearchDialogLastnameClicked),
        (gpointer)lastnameEntry
    );

    gtk_box_append(searchByLastNameBox, GTK_WIDGET(lastnameLabel));
    gtk_box_append(searchByLastNameBox, GTK_WIDGET(lastnameEntry));
    gtk_box_append(searchByLastNameBox, GTK_WIDGET(lastnameButton));
    gtk_box_append(mainBox, GTK_WIDGET(searchByLastNameBox));

    GtkBox *searchByCouriersLastNameBox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4));
    GtkLabel *couriersLastnameLabel = GTK_LABEL(gtk_label_new("Поиск по фамилии курьера: "));
    gtk_widget_set_margin_start(GTK_WIDGET(couriersLastnameLabel), 4);
    gtk_widget_set_margin_end(GTK_WIDGET(couriersLastnameLabel), 4);
    gtk_widget_set_margin_top(GTK_WIDGET(couriersLastnameLabel), 4);
    gtk_widget_set_margin_bottom(GTK_WIDGET(couriersLastnameLabel), 4);

    GtkEntry *couriersLastnameEntry = GTK_ENTRY(gtk_entry_new());
    gtk_widget_set_hexpand(GTK_WIDGET(couriersLastnameEntry), TRUE);
    gtk_widget_set_margin_start(GTK_WIDGET(couriersLastnameEntry), 4);
    gtk_widget_set_margin_end(GTK_WIDGET(couriersLastnameEntry), 4);
    gtk_widget_set_margin_top(GTK_WIDGET(couriersLastnameEntry), 4);
    gtk_widget_set_margin_bottom(GTK_WIDGET(couriersLastnameEntry), 4);

    GtkButton *couriersLastnameButton = GTK_BUTTON(gtk_button_new_with_label("Поиск"));
    gtk_widget_set_hexpand(GTK_WIDGET(couriersLastnameButton), TRUE);
    gtk_widget_set_margin_start(GTK_WIDGET(couriersLastnameButton), 4);
    gtk_widget_set_margin_end(GTK_WIDGET(couriersLastnameButton), 4);
    gtk_widget_set_margin_top(GTK_WIDGET(couriersLastnameButton), 4);
    gtk_widget_set_margin_bottom(GTK_WIDGET(couriersLastnameButton), 4);

    g_signal_connect(
        couriersLastnameButton, 
        "clicked", 
        G_CALLBACK(onSearchDialogCouriersLastnameClicked),
        (gpointer)couriersLastnameEntry
    );

    gtk_box_append(searchByCouriersLastNameBox, GTK_WIDGET(couriersLastnameLabel));
    gtk_box_append(searchByCouriersLastNameBox, GTK_WIDGET(couriersLastnameEntry));
    gtk_box_append(searchByCouriersLastNameBox, GTK_WIDGET(couriersLastnameButton));
    gtk_box_append(mainBox, GTK_WIDGET(searchByCouriersLastNameBox));

    GtkBox *searchByStatus = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4));
    GtkLabel *statusLabel_ = GTK_LABEL(gtk_label_new("Поиск по статусу заказа: "));
    gtk_widget_set_margin_start(GTK_WIDGET(statusLabel_), 4);
    gtk_widget_set_margin_end(GTK_WIDGET(statusLabel_), 4);
    gtk_widget_set_margin_top(GTK_WIDGET(statusLabel_), 4);
    gtk_widget_set_margin_bottom(GTK_WIDGET(statusLabel_), 4);

    GtkEntry *statusEntry = GTK_ENTRY(gtk_entry_new());
    gtk_widget_set_hexpand(GTK_WIDGET(statusEntry), TRUE);
    gtk_widget_set_margin_start(GTK_WIDGET(statusEntry), 4);
    gtk_widget_set_margin_end(GTK_WIDGET(statusEntry), 4);
    gtk_widget_set_margin_top(GTK_WIDGET(statusEntry), 4);
    gtk_widget_set_margin_bottom(GTK_WIDGET(statusEntry), 4);
    
    GtkButton *statusButton = GTK_BUTTON(gtk_button_new_with_label("Поиск"));
    gtk_widget_set_hexpand(GTK_WIDGET(statusButton), TRUE);
    gtk_widget_set_margin_start(GTK_WIDGET(statusButton), 4);
    gtk_widget_set_margin_end(GTK_WIDGET(statusButton), 4);
    gtk_widget_set_margin_top(GTK_WIDGET(statusButton), 4);
    gtk_widget_set_margin_bottom(GTK_WIDGET(statusButton), 4);
    
    g_signal_connect(
        statusButton, 
        "clicked", 
        G_CALLBACK(onSearchDialogStatusClicked),
        (gpointer)statusEntry
    );

    gtk_box_append(searchByStatus, GTK_WIDGET(statusLabel_));
    gtk_box_append(searchByStatus, GTK_WIDGET(statusEntry));
    gtk_box_append(searchByStatus, GTK_WIDGET(statusButton));
    gtk_box_append(mainBox, GTK_WIDGET(searchByStatus));

    GtkBox *searchByPizza = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4));
    GtkLabel *pizzaLabel = GTK_LABEL(gtk_label_new("Поиск по пицце: "));
    gtk_widget_set_margin_start(GTK_WIDGET(pizzaLabel), 4);
    gtk_widget_set_margin_end(GTK_WIDGET(pizzaLabel), 4);
    gtk_widget_set_margin_top(GTK_WIDGET(pizzaLabel), 4);
    gtk_widget_set_margin_bottom(GTK_WIDGET(pizzaLabel), 4);

    GtkEntry *pizzaEntry = GTK_ENTRY(gtk_entry_new());
    gtk_widget_set_hexpand(GTK_WIDGET(pizzaEntry), TRUE);
    gtk_widget_set_margin_start(GTK_WIDGET(pizzaEntry), 4);
    gtk_widget_set_margin_end(GTK_WIDGET(pizzaEntry), 4);
    gtk_widget_set_margin_top(GTK_WIDGET(pizzaEntry), 4);
    gtk_widget_set_margin_bottom(GTK_WIDGET(pizzaEntry), 4);

    GtkButton *pizzaButton = GTK_BUTTON(gtk_button_new_with_label("Поиск"));
    gtk_widget_set_hexpand(GTK_WIDGET(pizzaButton), TRUE);
    gtk_widget_set_margin_start(GTK_WIDGET(pizzaButton), 4);
    gtk_widget_set_margin_end(GTK_WIDGET(pizzaButton), 4);
    gtk_widget_set_margin_top(GTK_WIDGET(pizzaButton), 4);
    gtk_widget_set_margin_bottom(GTK_WIDGET(pizzaButton), 4);

    g_signal_connect(
        pizzaButton, 
        "clicked", 
        G_CALLBACK(onSearchDialogPizzaClicked),
        (gpointer)pizzaEntry
    );

    gtk_box_append(searchByPizza, GTK_WIDGET(pizzaLabel));
    gtk_box_append(searchByPizza, GTK_WIDGET(pizzaEntry));
    gtk_box_append(searchByPizza, GTK_WIDGET(pizzaButton));
    gtk_box_append(mainBox, GTK_WIDGET(searchByPizza));

    g_signal_connect(
        dialog, 
        "response", 
        G_CALLBACK(gtk_window_destroy), 
        NULL
    );

    gtk_box_append(GTK_BOX(contentArea), GTK_WIDGET(mainBox));
    gtk_widget_show(GTK_WIDGET(dialog));
}

static void onSortDialogSortByTimeClicked(GtkWidget *widget, gpointer data)
{
    stateSortOrdersByTime();
    renderUI();
}

static void onSortDialogSortByStatusClicked(GtkWidget *widget, gpointer data)
{
    stateSortOrdersByStatus();
    renderUI();
}

static void onSortDialogSortByPriceClicked(GtkWidget *widget, gpointer data)
{
    stateSortOrdersByPrice();
    renderUI();
}

static void onSortClicked(GtkWidget* widget, gpointer data)
{
    GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL;
    GtkWidget *contentArea;
    GtkDialog *dialog = GTK_DIALOG(gtk_dialog_new_with_buttons(
        "Сортировка заказов",
        GTK_WINDOW(window),
        flags,
        "OK",
        GTK_RESPONSE_NONE,
        NULL
    ));
    contentArea = gtk_dialog_get_content_area(dialog);

    GtkBox *mainBox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 4));

    GtkButton *sortByTime = GTK_BUTTON(gtk_button_new_with_label("Сортировать по времени заказа"));
    gtk_widget_set_margin_start(GTK_WIDGET(sortByTime), 4);
    gtk_widget_set_margin_end(GTK_WIDGET(sortByTime), 4);
    gtk_widget_set_margin_top(GTK_WIDGET(sortByTime), 4);
    gtk_widget_set_margin_bottom(GTK_WIDGET(sortByTime), 4);

    g_signal_connect(
        sortByTime, 
        "clicked", 
        G_CALLBACK(onSortDialogSortByTimeClicked),
        NULL
    );

    gtk_box_append(mainBox, GTK_WIDGET(sortByTime));

    GtkButton *sortByStatus = GTK_BUTTON(gtk_button_new_with_label("Сортировать по статусу заказа"));
    gtk_widget_set_margin_start(GTK_WIDGET(sortByStatus), 4);
    gtk_widget_set_margin_end(GTK_WIDGET(sortByStatus), 4);
    gtk_widget_set_margin_top(GTK_WIDGET(sortByStatus), 4);
    gtk_widget_set_margin_bottom(GTK_WIDGET(sortByStatus), 4);

    g_signal_connect(
        sortByStatus, 
        "clicked", 
        G_CALLBACK(onSortDialogSortByStatusClicked),
        NULL
    );

    gtk_box_append(mainBox, GTK_WIDGET(sortByStatus));

    GtkButton *sortByPrice = GTK_BUTTON(gtk_button_new_with_label("Сортировать по цене заказа"));
    gtk_widget_set_margin_start(GTK_WIDGET(sortByPrice), 4);
    gtk_widget_set_margin_end(GTK_WIDGET(sortByPrice), 4);
    gtk_widget_set_margin_top(GTK_WIDGET(sortByPrice), 4);
    gtk_widget_set_margin_bottom(GTK_WIDGET(sortByPrice), 4);

    g_signal_connect(
        sortByPrice, 
        "clicked", 
        G_CALLBACK(onSortDialogSortByPriceClicked),
        NULL
    );

    gtk_box_append(mainBox, GTK_WIDGET(sortByPrice));

    g_signal_connect(
        dialog, 
        "response", 
        G_CALLBACK(gtk_window_destroy), 
        NULL
    );

    gtk_box_append(GTK_BOX(contentArea), GTK_WIDGET(mainBox));
    gtk_widget_show(GTK_WIDGET(dialog));
}

char **_dialogFields;
size_t _dialogCurrentID;
static void onDialogLastnameEdit(GtkWidget* widget, gpointer data)
{
    const char* text = getEntryText(GTK_ENTRY(widget));
    strcpy(_dialogFields[0], text);
}

static void onDialogPhoneEdit(GtkWidget* widget, gpointer data)
{
    const char* text = getEntryText(GTK_ENTRY(widget));
    strcpy(_dialogFields[1], text);
}

static void onDialogDeliveryTimeEdit(GtkWidget* widget, gpointer data)
{
    const char* text = getEntryText(GTK_ENTRY(widget));
    strcpy(_dialogFields[2], text);
}

static void onValidateEdit(GtkWidget* widget, gpointer data)
{
    if (_dialogFields[0][0] == 0 || _dialogFields[1][0] == 0) {
        return;
    }
    if (
            strlen(_dialogFields[0]) >= 2 && 
            strlen(_dialogFields[1]) >= 5 && 
            validateNumber(_dialogFields[1]) &&
            validateNumber(_dialogFields[2])
        ) {
        editOrderLastName(_dialogCurrentID, _dialogFields[0]);
        editOrderPhone(_dialogCurrentID, _dialogFields[1]);
        setOrderDelivered(_dialogCurrentID, atoi(_dialogFields[2]));
        
        renderUI();
        gtk_window_destroy(GTK_WINDOW(widget));
    }
}

static void onOrderEditClicked(GtkWidget* widget, gpointer data)
{
    int id = GPOINTER_TO_INT(data);
    _dialogCurrentID = id;

    GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL;
    GtkWidget *contentArea;
    GtkDialog *dialog = GTK_DIALOG(gtk_dialog_new_with_buttons(
        "Редактировать заказ",
        GTK_WINDOW(window),
        flags,
        "OK",
        GTK_RESPONSE_NONE,
        NULL
    ));
    contentArea = gtk_dialog_get_content_area(dialog);

    _dialogFields = malloc(sizeof(char*) * 3);
    _dialogFields[0] = calloc(64, 1);
    _dialogFields[1] = calloc(64, 1);
    _dialogFields[2] = calloc(64, 1);

    DetailedOrder *order = getOrder(id);

    GtkBox *mainBox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 4));

    GtkBox *lastnameBox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4));
    GtkLabel *lastnameLabel = GTK_LABEL(gtk_label_new("Фамилия: "));
    gtk_widget_set_margin_start(GTK_WIDGET(lastnameLabel), 4);
    gtk_widget_set_margin_end(GTK_WIDGET(lastnameLabel), 4);
    gtk_widget_set_margin_top(GTK_WIDGET(lastnameLabel), 4);
    gtk_widget_set_margin_bottom(GTK_WIDGET(lastnameLabel), 4);

    GtkEntry *lastnameEntry = GTK_ENTRY(gtk_entry_new());
    gtk_widget_set_hexpand(GTK_WIDGET(lastnameEntry), TRUE);
    gtk_widget_set_margin_start(GTK_WIDGET(lastnameEntry), 4);
    gtk_widget_set_margin_end(GTK_WIDGET(lastnameEntry), 4);
    gtk_widget_set_margin_top(GTK_WIDGET(lastnameEntry), 4);
    gtk_widget_set_margin_bottom(GTK_WIDGET(lastnameEntry), 4);
    g_signal_connect(
        lastnameEntry, 
        "changed", 
        G_CALLBACK(onDialogLastnameEdit),
        NULL
    );

    gtk_entry_buffer_set_text(
        gtk_entry_get_buffer(lastnameEntry),
        order->order.lastName,
        -1
    );

    gtk_box_append(lastnameBox, GTK_WIDGET(lastnameLabel));
    gtk_box_append(lastnameBox, GTK_WIDGET(lastnameEntry));

    GtkBox *phoneBox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4));
    GtkLabel *phoneLabel = GTK_LABEL(gtk_label_new("Телефон: "));
    gtk_widget_set_margin_start(GTK_WIDGET(phoneLabel), 4);
    gtk_widget_set_margin_end(GTK_WIDGET(phoneLabel), 4);
    gtk_widget_set_margin_top(GTK_WIDGET(phoneLabel), 4);
    gtk_widget_set_margin_bottom(GTK_WIDGET(phoneLabel), 4);

    GtkEntry *phoneEntry = GTK_ENTRY(gtk_entry_new());
    gtk_widget_set_hexpand(GTK_WIDGET(phoneEntry), TRUE);
    gtk_widget_set_margin_start(GTK_WIDGET(phoneEntry), 4);
    gtk_widget_set_margin_end(GTK_WIDGET(phoneEntry), 4);
    gtk_widget_set_margin_top(GTK_WIDGET(phoneEntry), 4);
    gtk_widget_set_margin_bottom(GTK_WIDGET(phoneEntry), 4);

    g_signal_connect(
        phoneEntry, 
        "changed", 
        G_CALLBACK(onDialogPhoneEdit),
        NULL
    );

    gtk_entry_buffer_set_text(
        gtk_entry_get_buffer(phoneEntry),
        order->order.phone,
        -1
    );

    GtkBox *deliveryTimeBox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4));
    GtkLabel *deliveryTimeLabel = GTK_LABEL(gtk_label_new("Время доставки(в минутах): "));
    gtk_widget_set_margin_start(GTK_WIDGET(deliveryTimeLabel), 4);
    gtk_widget_set_margin_end(GTK_WIDGET(deliveryTimeLabel), 4);
    gtk_widget_set_margin_top(GTK_WIDGET(deliveryTimeLabel), 4);
    gtk_widget_set_margin_bottom(GTK_WIDGET(deliveryTimeLabel), 4);

    GtkEntry *deliveryTimeEntry = GTK_ENTRY(gtk_entry_new());
    gtk_widget_set_hexpand(GTK_WIDGET(deliveryTimeEntry), TRUE);
    gtk_widget_set_margin_start(GTK_WIDGET(deliveryTimeEntry), 4);
    gtk_widget_set_margin_end(GTK_WIDGET(deliveryTimeEntry), 4);
    gtk_widget_set_margin_top(GTK_WIDGET(deliveryTimeEntry), 4);
    gtk_widget_set_margin_bottom(GTK_WIDGET(deliveryTimeEntry), 4);
    
    g_signal_connect(
        deliveryTimeEntry, 
        "changed", 
        G_CALLBACK(onDialogDeliveryTimeEdit),
        NULL
    );


    char deliveryTimeStr[32];
    sprintf(deliveryTimeStr, "%d", order->deliveryTime);
    gtk_entry_buffer_set_text(
        gtk_entry_get_buffer(deliveryTimeEntry),
        deliveryTimeStr,
        -1
    );

    gtk_box_append(deliveryTimeBox, GTK_WIDGET(deliveryTimeLabel));
    gtk_box_append(deliveryTimeBox, GTK_WIDGET(deliveryTimeEntry));

    gtk_box_append(phoneBox, GTK_WIDGET(phoneLabel));
    gtk_box_append(phoneBox, GTK_WIDGET(phoneEntry));

    gtk_box_append(mainBox, GTK_WIDGET(lastnameBox));
    gtk_box_append(mainBox, GTK_WIDGET(phoneBox));
    gtk_box_append(mainBox, GTK_WIDGET(deliveryTimeBox));

    gtk_box_append(GTK_BOX(contentArea), GTK_WIDGET(mainBox));

    g_signal_connect(
        dialog, 
        "response", 
        G_CALLBACK(onValidateEdit),
        NULL
    );

    gtk_widget_show(GTK_WIDGET(dialog));
}

void addDetailedOrder(DetailedOrder *order)
{
    GtkBox *box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 4));
    GtkBox *orderBox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4));

    char orderLabelBuffer[64];
    sprintf(orderLabelBuffer, "Заказ №%d", order->id);

    GtkWidget *orderLabel = gtk_label_new(orderLabelBuffer);
    gtk_widget_set_hexpand(orderLabel, TRUE);
    gtk_widget_set_margin_top(orderLabel, 4);
    gtk_widget_set_margin_bottom(orderLabel, 4);
    gtk_box_append(orderBox, orderLabel);
    
    GtkWidget *editButton = gtk_button_new_with_label("Редактировать");
    gtk_widget_set_margin_start(editButton, 4);
    gtk_widget_set_margin_end(editButton, 4);
    gtk_widget_set_margin_top(editButton, 4);
    gtk_widget_set_margin_bottom(editButton, 4);
    g_signal_connect(
        editButton, 
        "clicked", 
        G_CALLBACK(onOrderEditClicked), 
        (gpointer)(size_t)order->id
    );
    gtk_box_append(orderBox, editButton);

    GtkWidget *deleteButton = gtk_button_new_with_label("Удалить");
    gtk_widget_set_margin_start(deleteButton, 4);
    gtk_widget_set_margin_end(deleteButton, 4);
    gtk_widget_set_margin_top(deleteButton, 4);
    gtk_widget_set_margin_bottom(deleteButton, 4);
    g_signal_connect(
        deleteButton, 
        "clicked", 
        G_CALLBACK(onOrderDeleteClicked), 
        (gpointer)(size_t)order->id
    );
    gtk_box_append(orderBox, deleteButton);

    gtk_box_append(box, GTK_WIDGET(orderBox));

    for (int i = 0; i < order->order.ordersCount; i++) {
        char buffer[128];
        sprintf(buffer, "%s, %dшт.", order->order.orders[i].pizza, order->order.orders[i].count);

        GtkWidget *pizzaLabel = gtk_label_new(buffer);
        gtk_widget_set_margin_top(pizzaLabel, 4);
        gtk_widget_set_margin_bottom(pizzaLabel, 4);
        gtk_label_set_xalign(GTK_LABEL(pizzaLabel), 0);

        gtk_box_append(box, pizzaLabel);
    }

    char buffer[512];
    sprintf(
        buffer, 
        "Цена: %dгрн (%dгрн заказ, %dгрн доставка).", 
        order->orderPrice + order->deliveryPrice,
        order->orderPrice,
        order->deliveryPrice
    );

    GtkWidget *priceLabel = gtk_label_new(buffer);
    gtk_widget_set_margin_top(priceLabel, 4);
    gtk_widget_set_margin_bottom(priceLabel, 4);
    gtk_label_set_xalign(GTK_LABEL(priceLabel), 0);
    gtk_box_append(box, priceLabel);

    strcpy(buffer, "Заказчик: ");
    strcat(buffer, order->order.lastName);
    strcat(buffer, "\nТелефон: ");
    strcat(buffer, order->order.phone);
    strcat(buffer, "\nАдрес: ");
    strcat(buffer, order->order.street);
    char buffer_[64];
    sprintf(buffer_, ", дом %d квартира %d", order->order.building, order->order.apartment);
    strcat(buffer, buffer_);

    GtkWidget *addressLabel = gtk_label_new(buffer);
    gtk_widget_set_margin_top(addressLabel, 4);
    gtk_widget_set_margin_bottom(addressLabel, 4);
    gtk_label_set_xalign(GTK_LABEL(addressLabel), 0);
    gtk_box_append(box, addressLabel);

    char statusStr[32];
    if (order->orderStatus == Delivery) {
        strcpy(statusStr, "Доставляется");
    } else if (order->orderStatus == Delivered) {
        strcpy(statusStr, "Доставлен");
    }

    sprintf(buffer, "Статус: %s, ", statusStr);
    strcat(buffer, "Курьер: ");
    strcat(buffer, order->courier.firstName);
    strcat(buffer, " ");
    strcat(buffer, order->courier.lastName);

    GtkWidget *statusLabel = gtk_label_new(buffer);
    gtk_widget_set_margin_top(statusLabel, 4);
    gtk_widget_set_margin_bottom(statusLabel, 4);
    gtk_label_set_xalign(GTK_LABEL(statusLabel), 0);
    gtk_box_append(box, statusLabel);

    GtkWidget *date = gtk_label_new(ctime(&(order->orderAcceptedTime)));
    gtk_widget_set_margin_top(date, 4);
    gtk_widget_set_margin_bottom(date, 4);
    gtk_label_set_xalign(GTK_LABEL(date), 0);
    gtk_box_append(box, date);

    GtkWidget *listBoxRow = gtk_list_box_row_new();
    gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(listBoxRow), GTK_WIDGET(box));
    gtk_list_box_insert(GTK_LIST_BOX(ordersListWidget), listBoxRow, -1);

    static size_t orderId = 0;

    detailedOrders[detailedOrdersCount] = (PizzaDetailedOrder){
        .id = orderId,
        .row = GTK_LIST_BOX_ROW(listBoxRow)
    };

    orderId++;
    detailedOrdersCount++;
}

int renderCallback(void* data)
{
    renderUI();
    return 0;
}

void renderFromOutside()
{
    g_idle_add(renderCallback, NULL);
}

void renderUI()
{
    State state = getCurrentState();

    const char* currentLastName = getEntryText(GTK_ENTRY(lastName));
    const char* currentStatus = gtk_label_get_text(GTK_LABEL(statusLabel));
    const char* currentStreet = getEntryText(GTK_ENTRY(street));
    const char* currentBuilding = getEntryText(GTK_ENTRY(building));
    const char* currentApartment = getEntryText(GTK_ENTRY(apartment));
    const char* currentPhone = getEntryText(GTK_ENTRY(phone));

    if (!CmpStr(currentStatus, state.statusBuffer) && state.statusBuffer != NULL) {
        gtk_label_set_text(GTK_LABEL(statusLabel), (const char*)state.statusBuffer);
        if (strlen(state.statusBuffer) == 0) {
            gtk_widget_set_sensitive(orderButton, TRUE);
        } else {
            gtk_widget_set_sensitive(orderButton, FALSE);
        }
    }
    if (!CmpStr(currentLastName, state.tempLastName) && state.tempLastName != NULL) {
        gtk_entry_buffer_set_text(
            gtk_entry_get_buffer(GTK_ENTRY(lastName)),
            (const char*)state.tempLastName,
            -1
        );
    }
    if (!CmpStr(currentStreet, state.tempStreet) && state.tempStreet != NULL) {
        gtk_entry_buffer_set_text(
            gtk_entry_get_buffer(GTK_ENTRY(street)),
            (const char*)state.tempStreet,
            -1
        );
    }
    char buildingStr[32], apartmentStr[32];
    sprintf(buildingStr, "%d", state.tempBuilding);
    sprintf(apartmentStr, "%d", state.tempApartment);
    if (state.tempBuilding == -1) {
        gtk_entry_buffer_set_text(
            gtk_entry_get_buffer(GTK_ENTRY(building)),
            "",
            -1
        );
    } else if (!CmpStr(currentBuilding, buildingStr)) {
        gtk_entry_buffer_set_text(
            gtk_entry_get_buffer(GTK_ENTRY(building)),
            buildingStr,
            -1
        );
    }
    if (state.tempApartment == -1) {
        gtk_entry_buffer_set_text(
            gtk_entry_get_buffer(GTK_ENTRY(apartment)),
            "",
            -1
        );
    } else if (!CmpStr(currentApartment, apartmentStr)) {
        gtk_entry_buffer_set_text(
            gtk_entry_get_buffer(GTK_ENTRY(apartment)),
            apartmentStr,
            -1
        );
    }
    if (!CmpStr(currentPhone, state.tempPhone)) {
        gtk_entry_buffer_set_text(
            gtk_entry_get_buffer(GTK_ENTRY(phone)),
            (const char*)state.tempPhone,
            -1
        );
    }
    
    if (pizzaOrderLinesCount != state.tempPizzaOrderLinesCount) {
        deleteAllPizzaOrderLines();
        for (int i = 0; i < state.tempPizzaOrderLinesCount; i++) {
            addPizzaOrderLine(
                state.tempPizzaOrderLines[i].pizzaIndex, 
                state.tempPizzaOrderLines[i].pizzaCount
            );
        }
    }

    deleteAllDetailedOrders();
    for (int i = 0; i < state.ordersCount; i++) {
        addDetailedOrder(&(state.ordersList[i]));
    }
}

static void activate(GtkApplication *app)
{
    char *uiFile = mapDataPath("ui/gui.ui");
    builder = gtk_builder_new_from_file(uiFile);
    free(uiFile);

    window = (GtkWidget*)gtk_builder_get_object(builder, "window");
    lastName = (GtkWidget*)gtk_builder_get_object(builder, "lastName");
    street = (GtkWidget*)gtk_builder_get_object(builder, "street");
    building = (GtkWidget*)gtk_builder_get_object(builder, "building");
    apartment = (GtkWidget*)gtk_builder_get_object(builder, "apartment");
    phone = (GtkWidget*)gtk_builder_get_object(builder, "phone");
    addPizzas = (GtkWidget*)gtk_builder_get_object(builder, "addPizzas");
    pizzaSelectBox = (GtkWidget*)gtk_builder_get_object(builder, "pizzaSelectBox");
    statusLabel = (GtkWidget*)gtk_builder_get_object(builder, "statusLabel");
    orderButton = (GtkWidget*)gtk_builder_get_object(builder, "order");
    ordersListWidget = (GtkWidget*)gtk_builder_get_object(builder, "ordersListBox");
    sortButton = (GtkWidget*)gtk_builder_get_object(builder, "sortButton");
    searchButton = (GtkWidget*)gtk_builder_get_object(builder, "searchButton");

    g_signal_connect(orderButton, "clicked", G_CALLBACK(onOrderCallback), NULL);
    g_signal_connect(addPizzas, "clicked", G_CALLBACK(onAddPizzasCallback), NULL);
    g_signal_connect(sortButton, "clicked", G_CALLBACK(onSortClicked), NULL);
    g_signal_connect(searchButton, "clicked", G_CALLBACK(onSearchClicked), NULL);

    g_signal_connect(lastName, "changed", G_CALLBACK(onLastNameEditCallback), NULL);
    g_signal_connect(street, "changed", G_CALLBACK(onStreetEditCallback), NULL);
    g_signal_connect(building, "changed", G_CALLBACK(onBuildingEditCallback), NULL);
    g_signal_connect(apartment, "changed", G_CALLBACK(onApartmentEditCallback), NULL);
    g_signal_connect(phone, "changed", G_CALLBACK(onPhoneEditCallback), NULL);

    initialState();
    renderUI();

    gtk_window_set_title(GTK_WINDOW(window), "Pizza Ordering System");

    gtk_application_add_window(app, GTK_WINDOW(window));
    gtk_widget_show(window);
}

int app_main(int argc, char* argv[])
{
    AdwApplication* app = adw_application_new("com.wrnchf.Course", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    return g_application_run(G_APPLICATION(app), argc, argv);
}
