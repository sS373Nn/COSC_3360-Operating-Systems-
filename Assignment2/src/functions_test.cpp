#include "functions.hpp"

int main(int argc, char* argv[]){


    //Expected input: std::string "request/resturn_resource a b ... n" where n is the number of resource groups and each variable is the number requested/returned from/to that group
    std::string request = "request_resource 1 0 3 2";
    std::vector<int> parsed_request_info = parse_resource_request_return(request);

    std::cout << "TESTING parse_resource_request_return FUNCTION:\nRequest: " << request << std::endl;
    for(const int &resource : parsed_request_info){
        std::cout << resource << std::endl;
    }

    std::cout << "\nEND TEST FOR parse_resource_request_return\n\n";

    
    ////////////////
    ////////////////

    //Expected input: global_time;resource_return x y z;resourcex0;resourcex1;resourcey0;...
    std::string message = "15;resource_return 1 0 2 1;apple;church;hotel;airplane";
    int global_time = 7;
    
    std::vector<std::string> partitioned_message = partition_resource_return_message(message, global_time);

    std::cout << "TESTING partition_resource_return_message FUNCTION:" << std::endl;
    std::cout << "\nOriginal global_time: 7\nUpdated global_time: " << global_time << std::endl;
    std::cout << "\nMessage Parts:\n";
    for(const std::string &part : partitioned_message){
        std::cout << part << std::endl;
    }

    std::cout << "\nEND TEST FOR partition_resource_return_message\n\n";


    ////////////////
    ////////////////

    //Use the resources created by our previous test
    //Should be a vector with resource_return x y z, resource, resource, resource,...
    std::vector<std::vector<std::string>> sorted_resources = sorted_resource_return(partitioned_message);

    std::cout << "TESTING sorted_resource_return FUNCTION: " << std::endl;
    std::cout << "\nOriginal global_time: 7\nUpdated global_time: " << global_time << std::endl;
    std::cout << "\nMessage Parts:\n";
    for(const std::vector<std::string> &group : sorted_resources){
        std::cout << "GROUP BEGIN:\n";
        for(const std::string &resource : group){
            std::cout << resource << ",";
        }
        std::cout << "\nGROUP END\n\n";
    }

    std::cout << "\nEND TEST FOR sorted_resource_return\n\n";


    ////////////////
    ////////////////

    //This is tested inside our previous test, but why not test explicitly?
    //Expects a std::vector<std::string> of any string and size
    std::vector<std::string> test_this_vector = {"apples", "pears", "grape"};

    std::queue<std::string> our_new_queue = build_resource_queue_from_group(test_this_vector);

    std::cout << "TESTING build_resource_queue_from_group FUNCTION: " << std::endl;

    std::cout << "Vector should now be empty:\n";
    if(test_this_vector.size() == 0){
        std::cout << "YUP! It's empty\n";
    }

    std::cout << "Resources in our queue in order:\n";
    while(!our_new_queue.empty()){
        std::cout << our_new_queue.front() << ", ";
        our_new_queue.pop();
    }

    std::cout << "\nEND TEST FOR build_resource_queue_from_group\n\n";


    ////////////////
    ////////////////

    //This is tested inside our previous test, but why not test explicitly?
    //Expects a std::queue<std::string> of any string and size
    //We'll return these items back to the vector we removed them from last time
    
    our_new_queue.push("apples");
    our_new_queue.push("pears");
    our_new_queue.push("grape");
    
    
    return_remaining_resources_from_queue_to_vector(our_new_queue, test_this_vector);

    std::cout << "TESTING return_remaining_resources_from_queue_to_vector FUNCTION: " << std::endl;

    if(our_new_queue.empty()){
        std::cout << "Queue is empty as expected!\n";
    }

    std::cout << "Items in vector:\n";
    for(const std::string &item : test_this_vector){
        std::cout << item << ", ";
    }
    std::cout << "\nEnd vector contents\n";

    std::cout << "\nEND TEST FOR return_remaining_resources_from_queue_to_vector\n\n";

    ////////////////
    ////////////////

    //Test putting the new vector back into the old vector
    //std::vector<std::vector<std::string>> bankers_mock_vector = {{"peach"}, {"BOB", "TERRY"}, {}, {}};


    ////
    //FOLLOWING TESTS BUILT BY CHATGPT//
    //BUT ONLY THE TESTS//
    //NONE OF MY CODE IS CHAT CODE//
    //ONLY//
    //SPECIFICED// 
    //TESTS//
    ////

    //////////////////////////
    // TEST: build_resource_release_string
    //////////////////////////
    std::vector<std::vector<std::string>> test_resources_owned_1 = {
        {"banana", "apple"},
        {},
        {"screwdriver", "hammer", "wrench"}
    };

    std::string release_msg = build_resource_release_string(test_resources_owned_1);
    std::cout << "TESTING build_resource_release_string:\n";
    std::cout << "Expected format: release 2 0 3\n";
    std::cout << "Actual: " << release_msg << "\n";
    std::cout << "END TEST\n\n";

    //////////////////////////
    // TEST: find_amount_of_resources_in_each_group
    //////////////////////////
    std::vector<int> resource_counts = find_amount_of_resources_in_each_group(test_resources_owned_1);

    std::cout << "TESTING find_amount_of_resources_in_each_group:\n";
    std::cout << "Expected: 2, 0, 3\n";
    std::cout << "Actual: ";
    for (int count : resource_counts) {
        std::cout << count << ", ";
    }
    std::cout << "\nEND TEST\n\n";

    //////////////////////////
    // TEST: build_resource_return_string
    //////////////////////////
    // Copy again since previous test clears it
    std::vector<std::vector<std::string>> test_resources_owned_2 = {
        {"banana", "apple"},
        {},
        {"screwdriver", "hammer", "wrench"}
    };

    std::string return_msg = build_resource_return_string(test_resources_owned_2);

    std::cout << "TESTING build_resource_return_string:\n";
    std::cout << "Expected string with semicolons and all resources:\n";
    std::cout << "Actual: " << return_msg << "\n";

    std::cout << "Expect vector cleared:\n";
    bool is_cleared = true;
    for (const auto& group : test_resources_owned_2) {
        if (!group.empty()) {
            is_cleared = false;
            break;
        }
    }
    std::cout << (is_cleared ? "YUP! It's cleared\n" : "NOPE! It's not cleared\n");
    std::cout << "END TEST\n\n";

    ////
    //END CHATGPT BUILT TESTS//
    ////

    return 0;

}
