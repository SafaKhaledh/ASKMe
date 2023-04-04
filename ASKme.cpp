//============================================================================
// Name        : ASKme.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
using namespace std;

bool allow_anonymous_ques(int user_id);
map<string, int> last_session_info;

void load_last_session_info() {

	// called everytime we run the program file > map
	ifstream last_session("last_session_info.txt");
	if(!last_session){
		cout << "can't open last_session_info.txt";
		return;
	}
	string key, line;
	int value;
	// Access each line in last_session file
	while(getline(last_session, line)) {
		size_t colon_pos = line.find(':');
		// structure it like a map
		if(colon_pos != string::npos) {
			key = line.substr(0,colon_pos);
			value = stoi(line.substr(colon_pos + 1));
		}

		last_session_info[key] = value;
	}

	// testing if info was loaded successfully
	for(auto &item: last_session_info) {
		cout << item.first << ":" << item.second << "\n";
	}
}


struct Question {
	// problem when running program many times
    int question_id;
    bool is_answered = false;
    bool asked_anonymously = false;
	string question;
	string answer = "";
	string parent_question_id;
	int From_user_id;
	int To_user_id;


	Question() { };
	Question(string question_, int From_user_id_, int To_user_id_,string parent_question_id_, bool asked_anonymously_) {
		question = question_;
		From_user_id = From_user_id_;
		To_user_id = To_user_id_;
		parent_question_id = parent_question_id_;
		question_id = ++last_session_info["last_question_id"];
		asked_anonymously = asked_anonymously_;

	}

	// for testing, updating files
	void print_Question(ostream& output_stream) {
		output_stream<< question_id << "," << parent_question_id << "," << From_user_id << ","
			         << To_user_id << "," << is_answered << "," << asked_anonymously << ","
					 << question << "," << answer << "\n";
	}

	void print_feed_question() {
		if(parent_question_id != "-1")
			cout <<"Thread parent Question Id (" << parent_question_id << ")";
		cout <<"Question Id (" << question_id << ")";

		if(!asked_anonymously)
			cout <<" from user id(" << From_user_id <<")";

		cout<<" To user id(" << To_user_id << ")\t\tQuestion: "
			<< question << "\n";
		cout << "\tAnswer: " << answer << "\n";
	}

	void print_question_ToMe() const{
		if(parent_question_id != "-1")
			cout<<"\tThread: ";

		cout <<"Question Id (" << question_id << ")" ;

		if (!asked_anonymously)
			cout << " from user id(" << From_user_id << ")";

		cout << "\t\tQuestion: " << question << "\n";
		// Display answer if any
		if(is_answered) {
			if(parent_question_id != "-1")
				cout << "\tThread:\t";
			cout << "\tAnswer: " <<answer << "\n\n";

		}
		else
			cout << "\n";
	}

	 void print_question_FromMe() const{
		cout <<"Question Id (" << question_id << ")";
				if(!(allow_anonymous_ques(To_user_id)))
					cout << " !AQ"; // the other guy can see my id!
		cout <<" to user id(" << To_user_id <<")\t\tQuestion: "
			 << question << "\t";
		// Display answer if any
		if(is_answered) {
			cout << "Answer: " <<answer << "\n";
		}
		else
			cout << " Not Answered YET\n";
	}
};


bool compare_Questions(const Question &Question1, const Question &Question2) {
	if(Question1.question_id < Question2.question_id)
		return true;
	return false;
}

void Display_Edit_Question(Question &question) {
	cout <<"Question Id (" << question.question_id << ") ";
	if(!question.asked_anonymously)
		cout <<"from user id(" << question.From_user_id <<")";
	cout << "Question: " << question.question << "\n";
	// Display answer if any
	if(question.is_answered) {
		cout << "		Answer: " << question.answer << "\n\n";
		cout << "Warning: Already answered. Answer will be updated\n";
	}

	cout << "Enter answer: ";
	cin.ignore();
	getline(cin, question.answer);
	question.is_answered = true; // Now it is answered, consistency!
	return;
}

map<Question, vector<Question>, decltype(&compare_Questions)> parent_children_Questions(&compare_Questions);

void print_map() {
	for(auto &item: parent_children_Questions) {
		auto p_ques = item.first;
		auto &child_questions = item.second;
		p_ques.print_Question(cout);

		for (auto &child_ques: child_questions)
			child_ques.print_Question(cout);
	}
}

struct User {
    int user_id;
	string user_name;
	string password;
	string name;
	string email;
	bool allow_anonymous_questions;

	User() {
		user_name = password = name = email = "";
		allow_anonymous_questions = false;

		}

	User(string _user_name, string _password, string _name, // for signup
			 string _email, bool _allow_anonymous_questions) {
		user_id = ++last_session_info["last_user_id"];
		user_name = _user_name;
		password = _password;
		name = _name;
		email = _email;
		allow_anonymous_questions = _allow_anonymous_questions;
	}

	void print_user() {
			cout<<"ID: " << user_id << "\t\t" << "Name: " << name <<"\n";
	}

	void print_ques_to_me() {
		// Access each pair in the map
		for(const auto &parent_children_Question: parent_children_Questions) {
			const auto &p_ques = parent_children_Question.first;  // Review chat gpt for why const in these 2 lines
			const auto &children_questions = parent_children_Question.second;

			if(p_ques.To_user_id == user_id) { // if p_ques is to me, then its children questions also to me
				p_ques.print_question_ToMe();
				for(const auto &child_ques: children_questions) {
					child_ques.print_question_ToMe();
				}
			}
		}
	}

	void print_ques_from_me() {
		for(const auto &parent_children_Question: parent_children_Questions) {
			const auto &p_ques = parent_children_Question.first;
			const auto &children_questions = parent_children_Question.second;

			if(p_ques.From_user_id == user_id) // remove bracket for some reason in design
				p_ques.print_question_FromMe();
				for(const auto &child_ques: children_questions) {
					if(child_ques.From_user_id == user_id)  // it is not necessary that children questions are all from me
						child_ques.print_question_FromMe();
				}
		}
	}

	void Ask_question() {
			int To_user_id, Asked_anonymously;
			cout << "Enter user id or -1 to cancel: ";
			cin >> To_user_id;

			if(To_user_id == -1)
				return;


		// validation for user id check if allow anonymous questions
			if(! allow_anonymous_ques(To_user_id)) {

				cout << "Note: Anonymous questions are not allowed for this user\n";
			}
			else {
				cout << "Ask Anonymously 0 or 1: ";
				cin >> Asked_anonymously;
			}
			cout << "For thread question, Enter Question id or -1 for new question: ";
			string parent_question_id;
			cin >> parent_question_id;

			cout << "Enter question text: ";
			cin.ignore();
			string question_txt;
			getline(cin, question_txt); // we have spaces!

			Question question(question_txt, user_id, To_user_id, parent_question_id, Asked_anonymously);

     		// if it is a parent question, insert it as key in parent children questions map
			if(question.parent_question_id == "-1")
				parent_children_Questions[question] = vector<Question>();

			else {
				// if it is a child associate it with the appropriate struct, i.e. push it to the
				// vector corresponding to the right key struct
				for(auto &item: parent_children_Questions) {
					Question p_ques = item.first;
					// is it my parent?
					if(stoi(question.parent_question_id) == p_ques.question_id)
						parent_children_Questions[p_ques].push_back(question);
				}
		   }

			// test module
			print_map();
			return;
		}
	void Answer_question() {
			// Answer a question or edits its answer
			cout << "Enter Question id or -1 to cancel"; // if
			int ques_id;
			cin >> ques_id;

			if (ques_id == -1)
				return;
			for(auto &thread_question: parent_children_Questions) {
				auto p_ques = thread_question.first;
				auto &children_quest = thread_question.second;

				if(p_ques.To_user_id == user_id) {

					if(p_ques.question_id == ques_id) {
						Display_Edit_Question(p_ques);

						 //just edit map key information!
						parent_children_Questions.erase(thread_question.first);  // review chatgpt map key removal
						parent_children_Questions[p_ques] = children_quest;

						// test module
						print_map();
						return;
					}

					else
						for(auto &child_ques: children_quest)
							if(child_ques.question_id == ques_id) {
								Display_Edit_Question(child_ques);
								// test module
								print_map();
								return;
							}
			 }
		}

	}

	void Delete_question() {
		cout << "Enter Question id or -1 to cancel";
		int ques_id;
		cin>>ques_id;

		// Access each pair in our map
	   for(auto &thread_ques: parent_children_Questions) {
		   auto p_ques = thread_ques.first;
		   auto child_questions = thread_ques.second;

		  // check the privilege I can delete only the questions asked to me!
		  // check the keys
		   if (p_ques.To_user_id == user_id) {
			   if(p_ques.question_id == ques_id) {
				   parent_children_Questions.erase(p_ques);
				   return;

			   }
			   else {
				   // check the values in the vector
				   for(auto it = child_questions.begin(); it != child_questions.end();) {
					   if(it-> question_id == ques_id) {
						   it = child_questions.erase(it);
						   return;
					   }
					   ++it;
				   }
			   }
		  }
	 }
	 print_map(); // test_module
  }



};

void Load_ques_File() {
	// this code assumes that the questions are in order parent have its children after it
	/* question file > question map */
	string question_info;
	ifstream questions_input ("questions.txt", ios::in);
	if(!questions_input) {
		cout << "can't open questions.txt file";
		return;
	}

	Question cur_parent_question;
	while(getline(questions_input, question_info)) {
			istringstream iss(question_info);

			Question question;
			string is_answered, asked_anonymously;
			string question_id;
			string From_user_id, To_user_id;

			// Get the questions info from the file & load it into the question object in a proper way!
			// Note, file structure is comma separated!
			getline(iss, question_id, ',');
			question.question_id = stoi(question_id);

			getline(iss, question.parent_question_id, ',');
			getline(iss, From_user_id, ',');
			getline(iss, To_user_id, ',');

			question.From_user_id = stoi(From_user_id);
			question.To_user_id = stoi(To_user_id);

			getline(iss, is_answered, ',');
			if(is_answered == "1") // keep in mind default values in the  Question struct
				question.is_answered = true;

			getline(iss, asked_anonymously, ',');
			if(asked_anonymously == "1")
				question.asked_anonymously = true;

			getline(iss, question.question, ',');
			getline(iss, question.answer);

			// Loading parent Questions into the map   parent_question_id == "-1"
			if(question.parent_question_id == "-1") {
				parent_children_Questions[question] = vector<Question>(); // review chatgpt to know why this is valid!
				cur_parent_question = question;
			}

			// loading child Questions   parent_question_id != "-1"
			else
				parent_children_Questions[cur_parent_question].push_back(question);
	}


	// test load questions file into the map properly
	for(auto &thread_question: parent_children_Questions) {
			auto p_ques = thread_question.first;
			auto children_questions = thread_question.second;
			p_ques.print_Question(cout);
			for(auto &child_ques: children_questions)
				child_ques.print_Question(cout);
   }
}


void Update_questionsFile() {
	// After each operation edit the questions file
	ofstream questions_output("questions.txt", ios::out);
	if(!questions_output) {
		cout << "can't open users.txt file";
		return;
	}

	for(auto &thread_question: parent_children_Questions) {
		auto p_ques = thread_question.first;
		auto children_questions = thread_question.second;

		p_ques.print_Question(questions_output);
		for(auto &child_ques: children_questions)
			child_ques.print_Question(questions_output);
	}
	return;
}

void update_last_session_file() {
     //	map > file
	// called at the end of the current session
	ofstream cur_session_info("last_session_info.txt", ios::out);
	if(!cur_session_info) {
		cout << "can't open last_session_info.txt file";
		return;
	}
	// test
//	cout << "update last session file \n last_user_id: " << user.user_id << "\n";

	cur_session_info <<"last_user_id:" << last_session_info["last_user_id"] << "\n";
	cur_session_info << "last_question_id:" << last_session_info["last_question_id"];
}

vector<User> users;
 bool allow_anonymous_ques(int user_id) {
 	for(auto user: users) {
 		if(user.user_id == user_id) {
 			return user.allow_anonymous_questions;
 		}
 	}
 }

void load_usersFile() {
	string user_info;
	ifstream users_input ("users.txt", ios::in);
	if(!users_input) {
		cout << "can't open users.txt file";
		return;
	}

	while(getline(users_input, user_info)) {
		istringstream iss(user_info);
		User user; // default constructor we don't need to change user_id we take it from the file

		string user_id;

		getline(iss,user_id, ',');
		user.user_id = stoi(user_id);

		getline(iss, user.user_name, ',');

		getline(iss, user.password, ',');

		getline(iss, user.name, ',');

		getline(iss, user.email, ',');

		string allow_anonymous_question;
		getline(iss,allow_anonymous_question);
		user.allow_anonymous_questions = 1;

		if(allow_anonymous_question == "0")
			user.allow_anonymous_questions = 0;

		users.push_back(user);
	}
	// check if loading was done successfully into the vector
	for(const auto &user: users)
		cout <<user.user_id << " " << user.user_name << " " << user.password << " "
		     << user.name << " " << user.email << " " << user.allow_anonymous_questions
			 << "\n";
}

void Update_usersFile() {
	ofstream users_output("users.txt", ios::out);
	if(!users_output) {
		cout << "can't open users.txt file";
		return;
	}

	for(const auto &user: users) {
		users_output << user.user_id << "," << user.user_name << "," << user.password << ","
					 << user.name << "," << user.email << "," << user.allow_anonymous_questions
					 << "\n";
	}
}

void List_users_feed() {
	// List the questions that has answers so far
	for(auto &thread_ques: parent_children_Questions) {
		auto p_ques = thread_ques.first;
		auto children_questions = thread_ques.second;

		if(p_ques.is_answered == true) {
			p_ques.print_feed_question();
			for(auto &child_ques: children_questions)
				if(child_ques.is_answered == true)
					child_ques.print_feed_question();
		}
	}
}

void List_system_users() {
	for( auto &user: users) {
		user.print_user();
	}
}

User SignUp() {
	string user_name, password, name, email;
	bool allow_anonymous_questions;

	cout << "Enter user name. (no spaces) : ";
	cin  >> user_name;
	cout << "Enter password: ";
	cin  >> password;
	cout << "Enter name: ";
	cin  >> name;
	cout << "Enter email: ";
	cin  >> email;
	cout << "Allow anonymous questions? : (0 or 1)";
	cin >> allow_anonymous_questions;

	// create user id at this stage
	User new_user(user_name, password, name, email, allow_anonymous_questions); // this object has an id!
	cout << new_user.user_id << "\n";

	users.push_back(new_user);
	// check if it was added to vector users
	for(auto user: users)
		cout << user.user_id << " " << user.user_name << " " << user.password << " " << user.name
		     << " " << user.email << " " << user.allow_anonymous_questions << "\n";
	return new_user;
}


User Login() {

 	string user_name, password;
 	cout << "\nEnter user_name & password: ";
 	cin >> user_name >> password;

 	// Search users vector
 	for(User &user: users) {
 		if (user.user_name == user_name && user.password == password)
 			return user;
 	}


 	User user;
 	return user; // default user
 }

int AskMe() {
	// return an integer that represents what type of log operation was required
	 //does internal checks
	cout << "1: Login\n";
	cout << "2: Sign Up\n";
	int choice;

	do {
	cout << "Enter number in range 1-2: ";
	cin >> choice;
	}
	while (choice < 1 || choice > 2);

	return choice;
}



int Menu() {  // called after signup or login operation // it does internal checks
	int choice;
	cout << "Menu:\n";
	cout << "1: Print Questions To Me\n";
	cout << "2: Print Questions From Me\n";
	cout << "3: Answer Question\n";
	cout << "4: Delete Question\n";
	cout << "5: Ask Question\n";
	cout << "6: List System Users\n";
	cout << "7: Feed\n";
	cout << "8: Logout\n";
	do {
		cout << "Enter number in range 1-8: ";
		cin >> choice;

		cout << choice << "\n";
	}

	while (choice < 1 || choice > 8);
	return choice;

}

void Run(User user) {
	Load_ques_File();
	while(true) {
		int choice = Menu();
		if(choice == 1)
			user.print_ques_to_me();

		else if(choice == 2)
			user.print_ques_from_me();

		else if(choice == 3)
			user.Answer_question();

		else if(choice == 4)
			user.Delete_question();

		else if(choice == 5) {
			user.Ask_question();
			update_last_session_file(); // update last question id
		}
		else if(choice == 6) {
			List_system_users();
		}
		else if(choice == 7)
			List_users_feed();

		else if(choice == 8)
			return;

		Update_questionsFile(); // is there a better position than that? i think this is the best one!
	}
}

int main() {
	User user;
	int log_choice = AskMe();
	load_last_session_info(); // just in case call it here we need it before signup to set up user_ids properly
	load_usersFile(); // does internal checks 2 reasons to call it here

	if(log_choice == 1) {
	    user = Login();

		while(user.user_name == "") { // not found in the system!
			cout << "Invalid user name or password";
			user = Login();
		}

    }

	else {
		user = SignUp(); // does internal checks
		update_last_session_file(); // edit user_id no edit to questions
		Update_usersFile(); //

	}

	Run(user);
}
