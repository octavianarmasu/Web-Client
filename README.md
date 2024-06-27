### Copyright Armasu Octavian


## PCom - Homework 4 - Web Client

### Overview
This project implements a web client in C that interacts with a REST API. It 
builds on the functionality developed in Lab 9 of the PCom course. The client
can execute the following commands: `register`, `login`, `enter_library`, 
`get_books`, `get_book`, `add_book`, `delete_book`, `logout`, and `exit`.

### Program Flow
The client reads user commands and sends corresponding requests to the server.
Two state variables track whether the user is logged in and whether they have 
entered the library. The client validates commands based on these states. The
ok variable is used to see if the command is valid or not. In case it is not
valid, the client displays an `Invalid command` message. The client remains 
operational, allowing multiple commands until the user types `exit`.

### Commands Explanation
- **`register`**: Registers a new user with a username and password. Both fields
must not contain spaces.
- **`login`**: Logs in an existing user with a username and password. The fields
must not contain spaces. Displays an error for invalid credentials.
- **`enter_library`**: Grants access to the library. Requires the user to be
logged in.
- **`get_books`**: Retrieves a list of all books in the library. Requires the
user to be logged in and have entered the library.
- **`get_book`**: Fetches details of a book by its ID. Requires the user to be 
logged in and have entered the library. Validates the ID as a number.
- **`add_book`**: Adds a new book to the library. Requires the user to be logged
in and have entered the library. Validates all fields and ensures the page count
is a number.
- **`delete_book`**: Deletes a book by its ID. Requires the user to be logged in 
and have entered the library. Validates the ID as a number.
- **`logout`**: Logs out the user. Requires the user to be logged in.
- **`exit`**: Exits the program.

### JSON Management
The client uses the `parson` library to handle JSON data for communication with
the server. JSON objects are created, serialized, and parsed using `parson`. The
library was chosen for its ease of use and comprehensive documentation. To create
a JSON object, a `JSON_Value` is initialized, converted to a `JSON_Object`, and
key-value pairs are added. The object is then serialized to a string using
`json_serialize_to_string_pretty` before sending to the server.

### Usage
1. **Register**: To register a new user, type `register` and follow the prompts
to enter a username and password.
2. **Login**: To login, type `login` and enter your username and password when
prompted.
3. **Enter Library**: After logging in, type `enter_library` to access the library.
4. **Get Books**: Type `get_books` to retrieve a list of all books in the library.
5. **Get Book**: To get details of a specific book, type `get_book` and provide
the book ID when prompted.
6. **Add Book**: To add a new book, type `add_book` and follow the prompts to
enter book details.
7. **Delete Book**: To delete a book, type `delete_book` and provide the book ID
when prompted.
8. **Logout**: To logout, type `logout`.
9. **Exit**: To exit the program, type `exit`.
