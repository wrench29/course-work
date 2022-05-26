#ifndef GUI_STATE_H_
#define GUI_STATE_H_
#include "service.h"

typedef struct PizzaOrderLine {
    int pizzaIndex;
    int pizzaCount;
} PizzaOrderLine;

typedef struct State {
    // List of all orders
    DetailedOrder *ordersList;
    int ordersCount;
    // Making order status
    char statusBuffer[256];
    // Making order
    char tempLastName[40];
    char tempStreet[40];
    int tempBuilding;
    int tempApartment;
    char tempPhone[20];
    PizzaOrderLine tempPizzaOrderLines[20];
    int tempPizzaOrderLinesCount;
} State;

State getCurrentState();
void initialState();
void stateResetMakingOrder();
void stateSetStatus(const char* status);
void stateAddOrder(DetailedOrder newOrder);
void stateDeleteOrder(int index);
void stateChangeMakingOrderLastName(char* lastName);
void stateChangeMakingOrderStreet(char* street);
void stateChangeMakingOrderBuilding(int building);
void stateChangeMakingOrderApartment(int apartment);
void stateChangeMakingOrderPhone(char* phone);
void stateResetMakingOrderOrderLines();
void stateChangeMakingOrderOrderLineIndex(int pos, int orderIndex);
void stateChangeMakingOrderOrderLineCount(int pos, int pizzaCount);
void stateAddMakingOrderOrderLine(int orderIndex, int pizzaCount);
void stateRemoveMakingOrderOrderLine(int index);
Order getMakingOrder();
void stateResetOrders();
void stateChangeOrderStatus(int id, OrderStatus status);
void stateChangeOrderDeliveryPrice(int id, unsigned int price);
void stateChangeOrderLastName(int id, char* lastname);
void stateChangeOrderPhone(int id, char* phone);
void stateSortOrdersByTime();
void stateSortOrdersByStatus();
void stateSortOrdersByPrice();
void stateFilterByLastName(char* lastname);
void stateFilterByCourierLastName(char* lastname);
void stateFilterByStatus(OrderStatus status);
void stateFilterByPizza(char* pizza);

#endif
