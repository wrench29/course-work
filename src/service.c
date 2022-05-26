#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>

#include "util.h"
#include "gui.h"
#include "gui_state.h"
#include "service.h"

volatile time_t currentTime;
DetailedOrder* ordersList;
int ordersCount;
Courier* couriersList;
int couriersCount;
StockLine* stock;
int stockCount;

pthread_t timeThread;

void loadOrders() {
    FILE *file = fopen(mapDataPath("data/orders.bin"), "r");
    ordersList = NULL;
    if (file == NULL) {
        return;
    }

    DetailedOrder tempOrder;
    while (fread(&tempOrder, sizeof(DetailedOrder), 1, file)) {
        ordersCount++;
        ordersList = realloc(ordersList, sizeof(DetailedOrder) * ordersCount);
        ordersList[ordersCount - 1] = tempOrder;
        stateAddOrder(ordersList[ordersCount - 1]);
    }
    fclose(file);

    renderFromOutside();
}
void loadCouriers() {
    couriersList = malloc(sizeof(Courier) * 3);
    couriersCount = 3;
    strcpy(couriersList[0].lastName, "Ivanov");
    strcpy(couriersList[0].firstName, "Aleksandr");
    strcpy(couriersList[1].lastName, "Petrov");
    strcpy(couriersList[1].firstName, "Aleksey");
    strcpy(couriersList[2].lastName, "Sidorov");
    strcpy(couriersList[2].firstName, "Andrey");
}
void loadStock() {
    stock = malloc(sizeof(StockLine) * 6);
    stockCount = 6;
    strcpy(stock[0].pizzaName, "Margarita");
    stock[0].price = 100;
    strcpy(stock[1].pizzaName, "Capriciosa");
    stock[1].price = 150;
    strcpy(stock[2].pizzaName, "Quattro Stagioni");
    stock[2].price = 130;
    strcpy(stock[3].pizzaName, "Calzone");
    stock[3].price = 170;
    strcpy(stock[4].pizzaName, "Funghi");
    stock[4].price = 120;
    strcpy(stock[5].pizzaName, "Napolitana");
    stock[5].price = 140;
}

StockLine* getStock(int *size)
{
    *size = stockCount;
    return stock;
}

Courier* selectCourier()
{
    return couriersList + randomInt(0, couriersCount - 1);
}

StockLine* findStockLine(char* name)
{
    for (int i = 0; i < stockCount; i++) {
        if (CmpStr(stock[i].pizzaName, name)) {
            return stock + i;
        }
    }
    return NULL;
}

__uint32_t calculatePrice(Order* order)
{
    __uint32_t price = 0;
    for (int i = 0; i < order->ordersCount; i++) {
        int pizzaPrice = findStockLine(order->orders[i].pizza)->price;
        price += pizzaPrice * order->orders[i].count;
    }
    return price;
}

void timeDaemon() {
    while (1) {
        currentTime = currentTime + 60;
        sleep(1);
    }
}

void initPizzaService()
{
    currentTime = time(NULL);
    loadOrders();
    loadCouriers();
    loadStock();

    pthread_create(&timeThread, NULL, (void*)timeDaemon, NULL);
}

int genID()
{
    int found = 0;
    int id = 0; 
    while (!found) {
        found = 1;
        id = abs(rand() % 100000);
        for (int i = 0; i < ordersCount; i++) {
            if (ordersList[i].id == id) {
                found = 0;
                break;
            }
        }
    }
    return id;
}

void addOrder(Order order)
{
    if (ordersCount == 0) {
        ordersList = malloc(sizeof(DetailedOrder));
    } else {
        ordersList = realloc(ordersList, sizeof(DetailedOrder) * (ordersCount + 1));
    }
    
    ordersList[ordersCount].order = order;
    ordersList[ordersCount].orderStatus = Delivery;
    ordersList[ordersCount].orderPrice = calculatePrice(&order);
    ordersList[ordersCount].deliveryPrice = 
            ordersList[ordersCount].orderPrice > 500 ? 0 : 30;
    ordersList[ordersCount].courier = *selectCourier();
    ordersList[ordersCount].orderAcceptedTime = currentTime;
    ordersList[ordersCount].id = genID();

    stateAddOrder(ordersList[ordersCount++]);
    renderFromOutside();
}

void deleteOrder(int id)
{
    for (int i = 0; i < ordersCount; i++) {
        if (ordersList[i].id == id) {
            ordersCount--;
            for (int j = i; j < ordersCount; j++) {
                ordersList[j] = ordersList[j + 1];
            }
            ordersList = realloc(ordersList, sizeof(DetailedOrder) * ordersCount);
            break;
        }
    }
    stateDeleteOrder(id);
    renderFromOutside();
}

void editOrderPhone(int id, char* phone)
{
    for (int i = 0; i < ordersCount; i++) {
        if (ordersList[i].id == id) {
            strcpy(ordersList[i].order.phone, phone);
            
            stateChangeOrderPhone(id, phone);
            renderFromOutside();
            return;
        }
    }
}
void editOrderLastName(int id, char* lastName)
{
    for (int i = 0; i < ordersCount; i++) {
        if (ordersList[i].id == id) {
            strcpy(ordersList[i].order.lastName, lastName);

            stateChangeOrderLastName(id, lastName);
            renderFromOutside();
            return;
        }
    }
}
void setOrderDelivered(int id, int timeOfDelivery)
{
    for (int i = 0; i < ordersCount; i++) {
        if (ordersList[i].id == id) {
            ordersList[i].orderStatus = Delivered;
            ordersList[i].deliveryTime = timeOfDelivery;

            if (timeOfDelivery == 0) {
                ordersList[i].orderStatus = Delivery;
            } else if (timeOfDelivery > 60) {
                ordersList[i].deliveryPrice = 0;
                stateChangeOrderDeliveryPrice(id, 0);
            } else {
                ordersList[i].deliveryPrice = 30;
                stateChangeOrderDeliveryPrice(id, 30);
            }

            stateChangeOrderStatus(id, ordersList[i].orderStatus);
            renderFromOutside();
            return;
        }
    }
}

DetailedOrder* getOrder(int id)
{
    for (int i = 0; i < ordersCount; i++) {
        if (ordersList[i].id == id) {
            return ordersList + i;
        }
    }
    return NULL;
}

DetailedOrder* getOrders(int *size)
{
    *size = ordersCount;
    return ordersList;
}

void destroyPizzaService()
{
    pthread_cancel(timeThread);

    FILE *ordersFile = fopen(mapDataPath("data/orders.bin"), "w");
    for (int i = 0; i < ordersCount; i++) {
        fwrite(&ordersList[i], sizeof(DetailedOrder), 1, ordersFile);
    }
    fclose(ordersFile);

    free(ordersList);
    free(couriersList);
    free(stock);
}
