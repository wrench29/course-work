#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "util.h"
#include "gui_state.h"

typedef struct Filter {
    int enabled;
    char lastName[40];         // 1
    char courierLastName[40];  // 2
    char pizza[40];            // 3
    OrderStatus status;        // 4
} Filter;

State currentState;
Filter filter = {
    .enabled = 0
};
int sorting = 0;

int _compByTime(const void* ptr1, const void* ptr2) {
    const DetailedOrder* order1 = ptr1;
    const DetailedOrder* order2 = ptr2;
    return order1->orderAcceptedTime > order2->orderAcceptedTime;
}

int _compByStatus(const void* ptr1, const void* ptr2) {
    const DetailedOrder* order1 = ptr1;
    const DetailedOrder* order2 = ptr2;
    return order1->orderStatus > order2->orderStatus;
}

int _compByPrice(const void* ptr1, const void* ptr2) {
    const DetailedOrder* order1 = ptr1;
    const DetailedOrder* order2 = ptr2;
    return order1->orderPrice < order2->orderPrice;
}

int _filterByLastName(DetailedOrder* order) {
    return strstr(order->order.lastName, filter.lastName) != NULL;
}

int _filterByCourierLastName(DetailedOrder* order) {
    return strstr(order->courier.lastName, filter.courierLastName) != NULL;
}

int _filterByStatus(DetailedOrder* order) {
    return order->orderStatus == filter.status;
}

int _filterByPizza(DetailedOrder* order) {
    return strstr(order->order.orders[0].pizza, filter.pizza) != NULL;
}

State getCurrentState()
{
    switch (sorting) {
        case 1:
            qsort(currentState.ordersList, currentState.ordersCount, sizeof(DetailedOrder), _compByTime);
            break;
        case 2:
            qsort(currentState.ordersList, currentState.ordersCount, sizeof(DetailedOrder), _compByStatus);
            break;
        case 3:
            qsort(currentState.ordersList, currentState.ordersCount, sizeof(DetailedOrder), _compByPrice);
            break;
    }

    if (!filter.enabled) {
        return currentState;
    }
    
    State newState = currentState;
    newState.ordersList = malloc(sizeof(DetailedOrder) * currentState.ordersCount);
    for (int i = 0; i < currentState.ordersCount; i++) {
        newState.ordersList[i] = currentState.ordersList[i];
    }
    newState.ordersCount = 0;

    switch (filter.enabled)
    {
    case 1:
        for (int i = 0; i < currentState.ordersCount; i++) {
            if (_filterByLastName(currentState.ordersList + i)) {
                newState.ordersList[newState.ordersCount++] = currentState.ordersList[i];
            }
        }
        break;
    case 2:
        for (int i = 0; i < currentState.ordersCount; i++) {
            if (_filterByCourierLastName(currentState.ordersList + i)) {
                newState.ordersList[newState.ordersCount++] = currentState.ordersList[i];
            }
        }
        break;
    case 3:
        for (int i = 0; i < currentState.ordersCount; i++) {
            if (_filterByStatus(currentState.ordersList + i)) {
                newState.ordersList[newState.ordersCount++] = currentState.ordersList[i];
            }
        }
        break;
    case 4:
        for (int i = 0; i < currentState.ordersCount; i++) {
            if (_filterByPizza(currentState.ordersList + i)) {
                newState.ordersList[newState.ordersCount++] = currentState.ordersList[i];
            }
        }
        break;
    }

    return newState;
}

void validateOrder()
{
    int status = 1;

    if (strlen(currentState.tempLastName) < 2)
    {
        strcpy(currentState.statusBuffer, "Фамилия не может быть меньше 2 букв");
        status = 0;
    }
    else if (strlen(currentState.tempStreet) < 2)
    {
        strcpy(currentState.statusBuffer, "Улица не может быть меньше 2 букв");
        status = 0;
    }
    else if (currentState.tempBuilding < 1)
    {
        strcpy(currentState.statusBuffer, "Номер дома должен состоять только из цифр, "
                                          "и не может быть меньше 1");
        status = 0;
    }
    else if (currentState.tempApartment < 1)
    {
        strcpy(currentState.statusBuffer, "Номер квартиры должен состоять только из цифр, "
                                          "и не может быть меньше 1");
        status = 0;
    }
    else if (
        strlen(currentState.tempPhone) < 5 ||
        !validateNumber(currentState.tempPhone))
    {
        strcpy(currentState.statusBuffer, "Номер телефона должен состоять только из цифр, "
                                          "и больше 5 цифр");
        status = 0;
    }
    else if (currentState.tempPizzaOrderLinesCount < 1)
    {
        strcpy(currentState.statusBuffer, "Вы не выбрали пиццу");
        status = 0;
    }
    else
    {
        for (int i = 0; i < currentState.tempPizzaOrderLinesCount; i++)
        {
            if (currentState.tempPizzaOrderLines[i].pizzaIndex < 0)
            {
                strcpy(currentState.statusBuffer, "Вы должны выбрать пиццу");
                status = 0;
                break;
            }
            if (currentState.tempPizzaOrderLines[i].pizzaCount < 1)
            {
                strcpy(currentState.statusBuffer, "Количество пиццы не может быть меньше 1");
                status = 0;
                break;
            }
        }
    }

    if (status)
    {
        strcpy(currentState.statusBuffer, "");
    }
}

void initialState()
{
    currentState.tempLastName[0] = '\0';
    currentState.tempStreet[0] = '\0';
    currentState.tempBuilding = -1;
    currentState.tempApartment = -1;
    currentState.tempPhone[0] = '\0';
    currentState.tempPizzaOrderLinesCount = 0;
    currentState.statusBuffer[0] = '\0';

    validateOrder();
}

void stateResetMakingOrder()
{
    currentState.tempLastName[0] = '\0';
    currentState.tempStreet[0] = '\0';
    currentState.tempBuilding = -1;
    currentState.tempApartment = -1;
    currentState.tempPhone[0] = '\0';
    currentState.tempPizzaOrderLinesCount = 0;
    currentState.statusBuffer[0] = '\0';

    validateOrder();
}

void stateSetStatus(const char *status)
{
    strcpy(currentState.statusBuffer, status);
}

void stateAddOrder(DetailedOrder newOrder)
{
    currentState.ordersCount++;
    if (currentState.ordersCount == 1) {
        currentState.ordersList = malloc(sizeof(DetailedOrder));
    } else {
        currentState.ordersList = realloc(
            currentState.ordersList, 
            currentState.ordersCount * sizeof(DetailedOrder)
        );
    }
    currentState.ordersList[currentState.ordersCount - 1] = newOrder;
}

void stateDeleteOrder(int index)
{
    for (int i = 0; i < currentState.ordersCount; i++)
    {
        if (currentState.ordersList[i].id == index)
        {
            for (int j = i; j < currentState.ordersCount - 1; j++)
            {
                currentState.ordersList[j] = currentState.ordersList[j + 1];
            }
            currentState.ordersCount--;
            break;
        }
    }
}

void stateChangeMakingOrderLastName(char *lastName)
{
    strcpy(currentState.tempLastName, lastName);
    validateOrder();
}

void stateChangeMakingOrderStreet(char *street)
{
    strcpy(currentState.tempStreet, street);
    validateOrder();
}

void stateChangeMakingOrderBuilding(int building)
{
    currentState.tempBuilding = building;
    validateOrder();
}

void stateChangeMakingOrderApartment(int apartment)
{
    currentState.tempApartment = apartment;
    validateOrder();
}

void stateChangeMakingOrderPhone(char *phone)
{
    strcpy(currentState.tempPhone, phone);
    validateOrder();
}

void stateResetMakingOrderOrderLines()
{
    currentState.tempPizzaOrderLinesCount = 0;
    validateOrder();
}

void stateChangeMakingOrderOrderLineIndex(int pos, int orderIndex)
{
    if (currentState.tempPizzaOrderLinesCount > pos)
    {
        currentState.tempPizzaOrderLines[pos].pizzaIndex = orderIndex;
        validateOrder();
    }
}

void stateChangeMakingOrderOrderLineCount(int pos, int pizzaCount)
{
    if (currentState.tempPizzaOrderLinesCount > pos)
    {
        currentState.tempPizzaOrderLines[pos].pizzaCount = pizzaCount;
        validateOrder();
    }
}

void stateAddMakingOrderOrderLine(int orderIndex, int pizzaCount)
{
    if (currentState.tempPizzaOrderLinesCount < 20)
    {
        currentState.tempPizzaOrderLines[currentState.tempPizzaOrderLinesCount] = (PizzaOrderLine){
            .pizzaIndex = orderIndex,
            .pizzaCount = pizzaCount,
        };
        currentState.tempPizzaOrderLinesCount++;
    }
    validateOrder();
}

void stateRemoveMakingOrderOrderLine(int index)
{
    for (int i = 0; i < currentState.tempPizzaOrderLinesCount; i++)
    {
        if (i == index)
        {
            for (int j = i; j < currentState.tempPizzaOrderLinesCount - 1; j++)
            {
                currentState.tempPizzaOrderLines[j] = currentState.tempPizzaOrderLines[j + 1];
            }
            currentState.tempPizzaOrderLinesCount--;
            break;
        }
    }
    validateOrder();
}

Order getMakingOrder()
{
    Order order;
    strcpy(order.lastName, currentState.tempLastName);
    strcpy(order.street, currentState.tempStreet);
    order.building = currentState.tempBuilding;
    order.apartment = currentState.tempApartment;
    strcpy(order.phone, currentState.tempPhone);
    order.ordersCount = currentState.tempPizzaOrderLinesCount;

    int stockSize = 0;
    StockLine *stock = getStock(&stockSize);
    char** pizzas = malloc(sizeof(char*) * stockSize);
    for (int i = 0; i < stockSize; i++) {
        pizzas[i] = malloc(64);
        strcpy(pizzas[i], stock[i].pizzaName);
    }

    for (int i = 0; i < currentState.tempPizzaOrderLinesCount; i++)
    {
        strcpy(order.orders[i].pizza, pizzas[currentState.tempPizzaOrderLines[i].pizzaIndex]);
        order.orders[i].count = currentState.tempPizzaOrderLines[i].pizzaCount;
    }
    for (int i = 0; i < stockSize; i++) {
        free(pizzas[i]);
    }
    free(pizzas);

    return order;
}

void stateChangeOrderStatus(int id, OrderStatus status)
{
    for (int i = 0; i < currentState.ordersCount; i++)
    {
        if (currentState.ordersList[i].id == id)
        {
            currentState.ordersList[i].orderStatus = status;
            break;
        }
    }
}

void stateChangeOrderDeliveryPrice(int id, unsigned int price)
{
    for (int i = 0; i < currentState.ordersCount; i++)
    {
        if (currentState.ordersList[i].id == id)
        {
            currentState.ordersList[i].deliveryPrice = price;
            break;
        }
    }
}

void stateChangeOrderLastName(int id, char* lastname)
{
    for (int i = 0; i < currentState.ordersCount; i++)
    {
        if (currentState.ordersList[i].id == id)
        {
            strcpy(currentState.ordersList[i].order.lastName, lastname);
            break;
        }
    }
}

void stateChangeOrderPhone(int id, char* phone)
{
    for (int i = 0; i < currentState.ordersCount; i++)
    {
        if (currentState.ordersList[i].id == id)
        {
            strcpy(currentState.ordersList[i].order.phone, phone);
            break;
        }
    }
}

void stateResetOrders()
{
    free(currentState.ordersList);
    currentState.ordersCount = 0;
}

void stateSortOrdersByTime() {
    sorting = 1;
}
void stateSortOrdersByStatus() {
    sorting = 2;
}
void stateSortOrdersByPrice() {
    sorting = 3;
}
void stateFilterByLastName(char* lastname) {
    filter.enabled = 1;
    strcpy(filter.lastName, lastname);
}
void stateFilterByCourierLastName(char* lastname) {
    filter.enabled = 2;
    strcpy(filter.courierLastName, lastname);
}
void stateFilterByStatus(OrderStatus status) {
    filter.enabled = 3;
    filter.status = status;
}
void stateFilterByPizza(char* pizza) {
    filter.enabled = 4;
    strcpy(filter.pizza, pizza);
}
