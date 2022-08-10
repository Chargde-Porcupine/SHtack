/**
 * @file webserver.c
 * @author Matthew Epshteing (epshteinmatthew@gmail.com)
 * @brief The server program for SHtack, the russian roulette for nerds
 * @version 1.0
 * @date 2022-08-07
 *
 * @copyright Copyright (c) 2022
 *
 * gcc -o webserver webserver.c -lulfius
 */
#include <stdio.h>  //inputs and outputs
#include <ulfius.h> //server library
#include <string.h> //string functions like strcat() and strcpy()
#include <malloc.h> //memory allocation
#include <time.h>   //self explanatory...

//vars
#define PORT 8000
#define NAME "Shtack"
#define VERSION "1.0"

struct linked_list_node
{
    char *item;
    struct linked_list_node *next;
};

struct linked_list_node *HEAD = NULL;

/**
 * @brief Santizes the command, removes sudo and &&
 * @param item A string that you wish to sanitize
 * @return A sanitized string, in const form to please the ulfius gods
 */
char *sanitize(const char *str)
{
    char *item = (char *)malloc(strlen(str) + 1 * sizeof(char));
    strcpy(item, str);

    char *cleaneditem = (char *)malloc(strlen(item) + 1 * sizeof(char));
    strcpy(cleaneditem, item);
    while (strstr(item, "sudo") != NULL || strstr(item, "su") != NULL)
    {
        strcpy(cleaneditem, (strstr(item, "su") ? (strstr(item, "do") ? strstr(item, "sudo") + 5 : strstr(item, "su") + 3) : item));
        strcpy(item, cleaneditem);
    }
    free(item);
    char *two = malloc(strlen(cleaneditem) + 1 * sizeof(char));
    do
    {
        strcpy(two, (strstr(cleaneditem, "&&")) ? strstr(cleaneditem, "&&") : "");
    } while (strcmp(cleaneditem, two) == 0 && strcmp(two, "") != 0);
    cleaneditem[strlen(cleaneditem) - (strlen(two))] = '\0';

    char *returned = strdup(cleaneditem);
    free(cleaneditem);
    free(two);
    return returned;
}

/**
 * @brief Pushes new item to back of linked list
 * @param item A string that you wish to insert to start of linked list
 * @return 0
 */
int push(char *item)
{
    struct linked_list_node *new_head = (struct linked_list_node *)malloc(sizeof(struct linked_list_node));
    new_head->item = strdup(item);
    new_head->next = HEAD;
    HEAD = new_head;
    return 0;
}

/**
 * @brief Pops last item of linked list
 * @return A string containing the popped item, or FAILURE if the linked list is empty
 */
char *pop()
{
    if(HEAD == NULL){
        return NULL; 
    }
    if (HEAD->next == NULL)
    {
      
        char *rtrnd = strdup(HEAD->item);
        HEAD = NULL;
        return rtrnd;
    }
    struct linked_list_node *current = HEAD;
    while (current->next->next != NULL)
    {
        current = current->next;
    } 
    char *rtrnd = strdup(current->next->item);
    current->next = NULL;
    return rtrnd;
}


/**
 * @brief Concatenates an array of strings.
 * @note Allocates memory for a new string. The user is responsible for freeing this string.
 * @param strs Strings to concat, must be terminated by an empty string.
 * @return A single string, the result of concatenating all of the input strings.
 */
char *strs_cat(const char **strs)
{
    char *output = malloc(1 * sizeof(char));

    size_t curr_len = 0;

    for (int i = 0; strs[i][0] != '\0'; ++i)
    {
        curr_len += strlen(strs[i]);
        output = realloc(output, curr_len * sizeof(char));
        if (i == 0)
        {
            strcpy(output, strs[i]);
            continue;
        }
        strcat(output, strs[i]);
    }

    return output;
}

char *get_time(void)
{
    time_t mytime = time(NULL);
    char *time_str = ctime(&mytime);
    time_str[strlen(time_str) - 1] = '\0';
    return time_str;
}

/**
 * Callback function for the web application on / url call, returns api name and version.
 */
int route(const struct _u_request *request, struct _u_response *response, void *user_data)
{
    char *responsetext = strs_cat((const char *[]){"You have reached the ", NAME, " API, version ", VERSION, ". ", "It is currently ", get_time(), " wherever this server is running.", ""});

    ulfius_set_string_body_response(response, 200, responsetext);
    free(responsetext);
    return U_CALLBACK_CONTINUE;
}

int route_push(const struct _u_request *request, struct _u_response *response, void *user_data)
{
    const char *c1 = sanitize(u_map_get(request->map_post_body, "command_1"));
    const char *c2 = sanitize(u_map_get(request->map_post_body, "command_2"));
    if (c1 == NULL || c2 == NULL)
    {
        ulfius_set_string_body_response(response, 400, "Not enough commands given. Try again.");
        return U_CALLBACK_CONTINUE;
    }
    push((char *) c1);
    push((char *) c2);
    ulfius_set_string_body_response(response, 303, "/pop");
    return U_CALLBACK_CONTINUE;
}

int route_pop(const struct _u_request *request, struct _u_response *response, void *user_data)
{
    char *popped = pop();

    if (popped == NULL)
    {
        ulfius_set_string_body_response(response, 500, "The SHtack is empty. /push to add to the Shtack.");
        return U_CALLBACK_CONTINUE;
    }
    ulfius_set_string_body_response(response, 200, popped);
    return U_CALLBACK_CONTINUE;
}

/**
 * main function
 */
int main(void)
{
    struct _u_instance instance;

    // Initialize instance with the port number
    if (ulfius_init_instance(&instance, PORT, NULL, NULL) != U_OK)
    {
        fprintf(stderr, "Error ulfius_init_instance, abort\n");
        return (1);
    }

    // Endpoint list declaration
    ulfius_add_endpoint_by_val(&instance, "GET", "/", NULL, 0, &route, NULL);

    ulfius_add_endpoint_by_val(&instance, "POST", "/push", NULL, 0, &route_push, NULL);

    ulfius_add_endpoint_by_val(&instance, "GET", "/pop", NULL, 0, &route_pop, NULL);

    // Start the framework
    if (ulfius_start_framework(&instance) == U_OK)
    {
        printf("Start framework on port %d\n", instance.port);

        // Wait for the user to press <enter> on the console to quit the application
        getchar();
    }
    else
    {
        fprintf(stderr, "Error starting framework\n");
    }
    printf("End framework\n");

    ulfius_stop_framework(&instance);
    ulfius_clean_instance(&instance);

    return 0;
}