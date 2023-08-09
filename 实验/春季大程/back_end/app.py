from flask import Flask, request, jsonify, session, make_response
import pymysql
import os

app = Flask(__name__)
app.config['SECRET_KEY'] = os.urandom(24)

DB = pymysql.connect(
    host='',
    user='',
    password='',
    database=''
)

cursor = DB.cursor(pymysql.cursors.DictCursor)


@app.route('/logout', methods=['get', 'post'])
def logout():
    remain = session['username']
    if remain is not None:
        print('logout:', remain)
        session.pop('username')
        session.pop('isAdmin')
        return jsonify({'request': 'succeeded', 'state': 'logout'})
    return jsonify({'request': 'failed', 'state': 'logout'})


@app.route('/login', methods=['post'])
def login():
    data = request.get_json(silent=False)
    print('login:', data['username'], data['password'])
    sql_str = '''SELECT * FROM card WHERE BINARY cno='{}' and password='{}' ''' \
        .format(data['username'], data['password'])
    cursor.execute(sql_str)
    res = cursor.fetchone()
    print('login: ', sql_str)
    print('login: ', res)
    if res:
        isAdmin = (res['type'] == 'O')
        session['cno'] = res['cno']
        session['username'] = res['name']
        session['isAdmin'] = isAdmin
        return jsonify({'request': 'succeeded', 'state': 'login', 'isAdmin': isAdmin})
    return jsonify({'request': 'failed', 'state': 'logout'})


@app.route('/checkLogin', methods=['get', 'post'])
def checkLogin():
    username = session.get('username')
    isAdmin = session.get('isAdmin')
    print('checkLogin:', username, 'isAdmin: ', isAdmin)
    if username is not None:
        return jsonify({'request': 'succeeded', 'state': 'login', 'username': username, 'isAdmin': isAdmin})
    else:
        return jsonify({'request': 'succeeded', 'state': 'logout'})


@app.route('/batch', methods=['post'])
def batch():
    requ_data = {
        'file': request.files.get('file'),
        'file_info': dict(request.form)
    }
    print(requ_data)
    text = requ_data['file'].read()
    print(text.decode('utf-8'))
    str = ''
    str += text.decode('utf-8')
    print(str)
    books = str.split("\r\n")
    print(books)
    for i in range(len(books)):
        try:
            sql_str = '''INSERT INTO book values{} '''.format(books[i])
            cursor.execute(sql_str)
            DB.commit()
        except:
            DB.rollback()
            return jsonify({'request': 'failed', 'reason': 'databaseError'})
    return jsonify({'request': 'succeeded'})


@app.route('/import', methods=['post'])
def Import():
    try:
        data = request.get_json(silent=False)
        sql_str = '''SELECT * from book WHERE bno='{}' '''.format(data['bno'])
        cursor.execute(sql_str)
        res = cursor.fetchall()
        if res:
            return jsonify({'request': 'failed', 'reason': 'duplicateBno'})
        sql_str = '''INSERT INTO book values('{}','{}','{}','{}','{}','{}','{}','{}','{}') ''' \
            .format(data['bno'], data['category'], data['title'], data['press'], data['year'], data['author'],
                    data['price'], data['total'], data['stock'])
        print("import:", sql_str)
        cursor.execute(sql_str)
        DB.commit()
        return jsonify({'request': 'succeeded'})
    except:
        DB.rollback()
        return jsonify({'request': 'failed', 'reason': 'databaseError'})


@app.route('/search', methods=['post'])
def search():
    data = request.get_json(silent=False)

    sql_str = '''SELECT bno, category, title, press, year, author, price, stock FROM book
                WHERE title like '%%{}%%' and category like '%%{}%%' and 
                    press like '%%{}%%' and author like '%%{}%%'
                '''.format(data['title'], data['category'], data['press'], data['author'])
    if data['yearFrom'] is not None:
        sql_str = sql_str + ''' and year>={} '''.format(data['yearFrom'])
    if data['yearTo'] is not None:
        sql_str = sql_str + ''' and year<={} '''.format(data['yearTo'])
    if data['priceFrom'] is not None:
        sql_str = sql_str + ''' and price>={} '''.format(data['priceFrom'])
    if data['priceTo'] is not None:
        sql_str = sql_str + ''' and price<={} '''.format(data['priceTo'])
    print('search: ', sql_str)
    cursor.execute(sql_str)
    res = cursor.fetchmany(50)
    print('result: ', res)
    return jsonify(res)


@app.route('/getborrow', methods=['post'])
def getBorrow():
    data = request.get_json(silent=False)
    if data['isAdmin'] is True:
        sql_str = '''SELECT bno FROM borrow
                       WHERE cno='{}' AND return_date is null
                       '''.format(data['cno'])
        print('search borrows:', sql_str)
        cursor.execute(sql_str)
        res = cursor.fetchmany(50)
        c = 0
        bres = []
        for i in range(0, len(res), 1):
            sql_str = '''SELECT * FROM book
                               WHERE bno='{}'
                               '''.format(res[i]['bno'])
            cursor.execute(sql_str)
            bres.insert(c, cursor.fetchone())
            print(len(bres))
            c = c + 1
        print('bres:', bres)
        return jsonify(bres)
    else:
        sql_str = '''SELECT bno FROM borrow
                               WHERE cno='{}' AND return_date is null
                               '''.format(session['cno'])
        print('search borrows 1:', sql_str)
        cursor.execute(sql_str)
        res = cursor.fetchmany(50)
        c = 0
        bres = []
        for i in range(0, len(res), 1):
            sql_str = '''SELECT * FROM book
                                       WHERE bno='{}'
                                       '''.format(res[i]['bno'])
            cursor.execute(sql_str)
            bres.insert(c, cursor.fetchone())
            print(len(bres))
            c = c + 1
        print('bres:', bres)
        return jsonify(bres)


@app.route('/borrow', methods=['post'])
def borrow():
    try:
        data = request.get_json(silent=False)
        sql_str = '''SELECT * FROM borrow WHERE BINARY cno='{}' and bno='{}' and return_date is null ''' \
            .format(data['borrowcno'], data['bno'])
        cursor.execute(sql_str)
        res = cursor.fetchone()
        if res is None:
            sql_str = '''UPDATE book SET stock={}
                            WHERE bno='{}'
                            '''.format(data['stock'] - 1, data['bno'])
            print('borrow: ', sql_str)
            cursor.execute(sql_str)
            sql_str = '''INSERT INTO borrow values('{}','{}',now() ,null,'{}',null)
                                '''.format(data['borrowcno'], data['bno'], session['cno'])
            print('card: ', sql_str)
            cursor.execute(sql_str)
            DB.commit()
            return jsonify({'request': 'succeeded'})
        else:
            return jsonify({'request': 'failed', 'reason': 'haveBorrowed'})
    except:
        DB.rollback()
        return jsonify({'request': 'failed', 'reason': 'databaseError'})


@app.route('/returnbook', methods=['post'])
def returnBook():
    try:
        # update stock
        data = request.get_json(silent=False)
        sql_str = '''UPDATE borrow SET return_date=now() WHERE BINARY cno='{}' and bno='{}' ''' \
            .format(data['cno'], data['bno'])
        cursor.execute(sql_str)
        sql_str = '''UPDATE book SET stock={}
                                WHERE bno='{}'
                                '''.format(data['stock'] + 1, data['bno'])
        print('borrow: ', sql_str)
        cursor.execute(sql_str)
        sql_str = '''UPDATE borrow SET return_admin='{}' WHERE BINARY cno='{}' and bno='{}' ''' \
            .format(session['cno'], data['cno'], data['bno'])
        cursor.execute(sql_str)
        DB.commit()
        # get borrowed books' info
        sql_str = '''SELECT bno FROM borrow
                           WHERE cno='{}' AND return_date is null
                           '''.format(data['cno'])
        print('search borrows:', sql_str)
        cursor.execute(sql_str)
        res = cursor.fetchmany(50)
        print('res:', res)
        bres = []
        for i in range(0, len(res), 1):
            sql_str = '''SELECT * FROM book
                                   WHERE bno='{}' '''.format(res[i]['bno'])
            cursor.execute(sql_str)
            bres.append(cursor.fetchone())
            print(len(bres))
        print('bres:', bres)
        return jsonify(bres)
    except:
        DB.rollback()
        return jsonify({'request': 'failed', 'reason': 'databaseError'})


@app.route('/manager', methods=['post'])
def id_manage():
    data = request.get_json(silent=False)

    sql_str = '''SELECT cno, name, department, type FROM card
                WHERE cno like '%%{}%%' and name like'%%{}%%' and department like '%%{}%%'
                '''.format(data['cno'], data['name'], data['department'])
    print('search card: ', sql_str)
    cursor.execute(sql_str)
    res = cursor.fetchmany(50)
    print('result: ', res)
    return jsonify(res)


@app.route('/delete_card', methods=['post'])
def delete_card():
    try:
        data = request.get_json(silent=False)
        print(data['cno'])
        sql_str = '''DELETE FROM card WHERE cno='{}' '''.format(data['cno'])
        print('delete card: ', sql_str)
        cursor.execute(sql_str)
        DB.commit()
        sql_str = '''SELECT cno, name, department FROM card
                        WHERE cno like '%%{}%%' and name like'%%{}%%' and department like '%%{}%%'
                    '''.format(data['search_cno'], data['name'], data['department'])
        cursor.execute(sql_str)
        res = cursor.fetchmany(50)
        return jsonify(res)
    except:
        DB.rollback()
        return jsonify({'request': 'failed', 'reason': 'databaseError'})


@app.route('/check_repeat', methods=['post'])
def check_repeat():
    try:
        data = request.get_json(silent=False)
        sql_str = '''SELECT cno FROM card WHERE cno='{}'
                '''.format(data['cno'])
        print('search repeat cno:', sql_str)
        cursor.execute(sql_str)
        res = cursor.fetchmany(50)
        print('len(res):', len(res))
        if len(res) == 0:
            return jsonify({'request': 'succeeded'})
        else:
            return jsonify({'request': 'failed', 'reason': 'repeatCno'})
    except:
        return jsonify({'request': 'failed', 'reason': 'databaseError'})


@app.route('/create_card', methods=['post'])
def create_card():
    try:
        data = request.get_json(silent=False)
        sql_str = '''INSERT INTO card(cno,name,department,type,password) values('{}','{}','{}','{}','{}')
        '''.format(data['cno'], data['name'], data['department'], data['type'], data['password'])
        print('insert new card into db:', sql_str)
        cursor.execute(sql_str)
        sql_str = '''SELECT cno, name, department FROM card
                        WHERE cno like '%%{}%%' and name like'%%{}%%' and department like '%%{}%%'
                        '''.format(data['cur_cno'], data['cur_name'], data['cur_department'])
        cursor.execute(sql_str)
        res = cursor.fetchmany(50)
        DB.commit()
        return jsonify(res)
    except:
        DB.rollback()
        return jsonify({'request': 'failed', 'reason': 'databaseError'})


if __name__ == '__main__':
    app.run()
