#ifndef SERVICE_H_
#define SERVICE_H_

#include <time.h>

typedef struct OrderLine {
    char pizza[40];
    int count;
} OrderLine;

typedef struct Order {
    char lastName[40];
    char street[40];
    int building;
    int apartment;
    char phone[20];
    OrderLine orders[20];
    int ordersCount;
} Order;

typedef enum OrderStatus {
    Delivery,
    Delivered
} OrderStatus;

typedef struct Courier {
    char lastName[40];
    char firstName[40];
} Courier;

typedef struct DetailedOrder {
    Order order;
    OrderStatus orderStatus;
    __uint32_t orderPrice;
    __uint32_t deliveryPrice;
    Courier courier;
    time_t orderAcceptedTime;
    int deliveryTime;
    int id;
} DetailedOrder;

typedef struct StockLine {
    char pizzaName[40];
    __uint32_t price;
} StockLine;

void initPizzaService();

StockLine* getStock(int *size);
void addOrder(Order order);
DetailedOrder* getOrder(int id);
void editOrderPhone(int id, char* phone);
void editOrderLastName(int id, char* lastName);
void setOrderDelivered(int id, int timeOfDelivery);
DetailedOrder* getOrders(int *size);
void deleteOrder(int id);

void destroyPizzaService();

#endif
