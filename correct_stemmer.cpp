#include <iostream>
#include <string>
using namespace std;

class Stemmer
{
public:
    string b;
    int i,     /* offset into b */
    i_end, /* offset to end of stemmed word */
    j, k;
    static const int INC = 50;
    /* unit of size whereby b is increased */

    Stemmer()
    {
        b = "";
        i = 0;
        i_end = 0;
    }

    /**
     * Add a character to the word being stemmed. When you are finished
     * adding characters, you can call stem() to stem the word.
     */

    void add(char ch)
    {
        if (i == b.length())
        {
            b.resize(i + INC);
        }
        b[i++] = ch;
    }

    /** Adds wLen characters to the word being stemmed contained in a portion
     * of a char[] array. This is like repeated calls of add(char ch), but
     * faster.
     */

    void add(string w, int wLen)
    {
        if (i + wLen >= b.length())
        {
            b.resize(i + wLen + INC);
        }
        for (int c = 0; c < wLen; c++)
            b[i++] = w[c];
    }

    /**
     * After a word has been stemmed, it can be retrieved by toString(),
     * or a reference to the internal buffer can be retrieved by getResultBuffer
     * and getResultLength (which is generally more efficient.)
     */
    string toString()
    {
        return b;
    }

    /**
     * Returns the length of the word resulting from the stemming process.
     */
    int getResultLength()
    {
        return i_end;
    }

    /**
     * Returns a reference to a character buffer containing the results of
     * the stemming process. You also need to consult getResultLength()
     * to determine the length of the result.
     */
    string getResultBuffer()
    {
        return b;
    }

    /* cons(i) is true <=> b[i] is a consonant. */
    bool cons(int i)
    {
        switch (b[i])
        {
            case 'a':
            case 'e':
            case 'i':
            case 'o':
            case 'u':
                return false;
            case 'y':
                return (i == 0) ? true : !cons(i - 1);
            default:
                return true;
        }
    }

    /* m() measures the number of consonant sequences between 0 and j. if c is
       a consonant sequence and v a vowel sequence, and <..> indicates arbitrary
       presence,

          <c><v>       gives 0
          <c>vc<v>     gives 1
          <c>vcvc<v>   gives 2
          <c>vcvcvc>v> gives 3
          ....
    */
    int m()
    {
        int n = 0;
        int i = 0;
        while (true)
        {
            if (i > j) return n;
            if (!cons(i)) break;
            i++;
        }
        i++;
        while (true)
        {
            while (true)
            {
                if (i > j) return n;
                if (cons(i)) break;
                i++;
            }
            i++;
            n++;
            while (true)
            {
                if (i > j) return n;
                if (!cons(i)) break;
                i++;
            }
            i++;
        }
    }

    /* vowelinstem() is true <=> 0,...j contains a vowel */
    bool vowelinstem()
    {
        for (int i = 0; i <= j; i++)
            if (!cons(i))
                return true;
        return false;
    }

    /* doublec(j) is true <=> j,(j-1) contain a double consonant. */
    bool doublec(int j)
    {
        if (j < 1)
            return false;
        if (b[j] != b[j - 1])
            return false;
        return cons(j);
    }

    /* cvc(i) is true <=> i-2,i-1,i has the form consonant - vowel - consonant
       and also if the second c is not w,x or y. this is used when trying to
       restore an e at the end of a short word. e.g.

          cav(e), lov(e), hop(e), crim(e), but
          snow, box, tray.

    */
    bool cvc(int i)
    {
        if (i < 2 || !cons(i) || cons(i - 1) || !cons(i - 2))
            return false;
        char ch = b[i];
        if (ch == 'w' || ch == 'x' || ch == 'y')
            return false;
        return true;
    }

    bool ends(string s)
    {
        int l = s.size();
        int o = k - l + 1;
        if (o < 0)
            return false;
        string sc = s;
        for (int i = 0; i < l; i++)
            if (b[o + i] != sc[i])
                return false;
        j = k - l;
        return true;
    }

    /* setto(s) sets (j+1),...k to the characters in the string s, readjusting
       k. */
    void setto(string s)
    {
       // cout<<"hi"<<endl;
        int l = s.size();
        int o = j + 1;
        string sc = s;
        for (int i = 0; i < l; i++)
            b[o + i] = sc[i];
        k = j + l;
    }

    /* r(s) is used further down. */
    void r(string s)
    {
        if (m() > 0)
            setto(s);
    }

    /* step1() gets rid of plurals and -ed or -ing. e.g.
           caresses  ->  caress
           ponies    ->  poni
           ties      ->  ti
           caress    ->  caress
           cats      ->  cat

           feed      ->  feed
           agreed    ->  agree
           disabled  ->  disable

           matting   ->  mat
           mating    ->  mate
           meeting   ->  meet
           milling   ->  mill
           messing   ->  mess

           meetings  ->  meet

    */

    void step1()
    {
        //cout<<"kk"<<endl;
        if (b[k] == 's')
        {
            //cout<<"yy"<<endl;
            if (ends("sses"))
                k -= 2;
            else if (ends("ies"))
                setto("i");
            else if (b[k - 1] != 's')
                k--;
        }
        if (ends("eed"))
        {
            if (m() > 0)
                k--;
        }
        else if ((ends("ed") || ends("ing")) && vowelinstem())
        {
            //cout<<"ll"<<endl;
            k = j;
           /// cout<<k<<endl;
            if (ends("at"))
                setto("ate");
            else if (ends("bl"))
                setto("ble");
            else if (ends("iz"))
                setto("ize");
            else if (doublec(k))
            {
                k--;
                char ch = b[k];
                if (ch == 'l' || ch == 's' || ch == 'z')
                    k++;
            }
            else if (m() == 1 && cvc(k)) setto("e");
        }
        //cout<<b<<endl;
    }

    /* step2() turns terminal y to i when there is another vowel in the stem. */
    void step2()
    {
        if (ends("y") && vowelinstem())
            b[k] = 'i';
    }

    /* step3() maps double suffices to single ones. so -ization ( = -ize plus
       -ation) maps to -ize etc. note that the string before the suffix must give
       m() > 0. */
    void step3()
    {
        if (k == 0)
            return;

        /* For Bug 1 */
        switch (b[k - 1])
        {
            case 'a':
                if (ends("ational")) { r("ate"); break; }
                if (ends("tional")) { r("tion"); break; }
                break;
            case 'c':
                if (ends("enci")) { r("ence"); break; }
                if (ends("anci")) { r("ance"); break; }
                break;
            case 'e':
                if (ends("izer")) { r("ize"); break; }
                break;
            case 'l':
                if (ends("bli")) { r("ble"); break; }
                if (ends("alli")) { r("al"); break; }
                if (ends("entli")) { r("ent"); break; }
                if (ends("eli")) { r("e"); break; }
                if (ends("ousli")) { r("ous"); break; }
                break;
            case 'o':
                if (ends("ization")) { r("ize"); break; }
                if (ends("ation")) { r("ate"); break; }
                if (ends("ator")) { r("ate"); break; }
                break;
            case 's':
                if (ends("alism")) { r("al"); break; }
                if (ends("iveness")) { r("ive"); break; }
                if (ends("fulness")) { r("ful"); break; }
                if (ends("ousness")) { r("ous"); break; }
                break;
            case 't':
                if (ends("aliti")) { r("al"); break; }
                if (ends("iviti")) { r("ive"); break; }
                if (ends("biliti")) { r("ble"); break; }
                break;
            case 'g':
                if (ends("logi")) { r("log"); break; }
                break;
            default:
                break;
        }
    }

    /* step4() deals with -ic-, -full, -ness etc. similar strategy to step3. */
    void step4()
    {
        switch (b[k])
        {
            case 'e':
                if (ends("icate")) { r("ic"); break; }
                if (ends("ative")) { r(""); break; }
                if (ends("alize")) { r("al"); break; }
                break;
            case 'i':
                if (ends("iciti")) { r("ic"); break; }
                break;
            case 'l':
                if (ends("ical")) { r("ic"); break; }
                if (ends("ful")) { r(""); break; }
                break;
            case 's':
                if (ends("ness")) { r(""); break; }
                break;
        }
    }

    /* step5() takes off -ant, -ence etc., in context <c>vcvc<v>. */
    void step5()
    {
        if (k == 0)
            return;

        /* for Bug 1 */
        switch (b[k - 1])
        {
            case 'a':
                if (ends("al")) break; return;
            case 'c':
                if (ends("ance")) break;
                if (ends("ence")) break; return;
            case 'e':
                if (ends("er")) break; return;
            case 'i':
                if (ends("ic")) break; return;
            case 'l':
                if (ends("able")) break;
                if (ends("ible")) break; return;
            case 'n':
                if (ends("ant")) break;
                if (ends("ement")) break;
                if (ends("ment")) break;
                /* element etc. not stripped before the m */
                if (ends("ent")) break; return;
            case 'o':
                if (ends("ion") && j >= 0 && (b[j] == 's' || b[j] == 't')) break;
                /* j >= 0 fixes Bug 2 */
                if (ends("ou")) break; return;
            /* takes care of -ous */
            case 's':
                if (ends("ism")) break; return;
            case 't':
                if (ends("ate")) break;
                if (ends("iti")) break; return;
            case 'u':
                if (ends("ous")) break; return;
            case 'v':
                if (ends("ive")) break; return;
            case 'z':
                if (ends("ize")) break; return;
            default:
                return;
        }
        if (m() > 1)
            k = j;
    }

    /* step6() removes a final -e if m() > 1. */
    void step6()
    {
        j = k;

        if (b[k] == 'e')
        {
            int a = m();
            if (a > 1 || (a == 1 && !cvc(k - 1)))
                k--;
        }
        if (b[k] == 'l' && doublec(k) && m() > 1)
            k--;
    }

    /** Stem the word placed into the Stemmer buffer through calls to add().
     * Returns true if the stemming process resulted in a word different
     * from the input. You can retrieve the result with
     * getResultLength()/getResultBuffer() or toString().
     */
    void stem()
    {
        //cout<<"hi"<<endl;
        k = i - 1;
        if (k > 1)
        {
            step1();
            step2();
            step3();
            step4();
            step5();
          
            step6();
        }
        i_end = k + 1;
        i = 0;
    }
};

// void display(const string& str) {
//     cout << "Stemmed: " << str << endl;
//     cout << "Length: " << str.length() << endl;
// }

void display(string c,int i){

    for(int j=0;j<i;j++)
    {
        cout<<c[j];
    }cout<<endl;

}


int main() {
    Stemmer stemmer;
    string word = "namans";
    stemmer.add(word, word.size());
    stemmer.stem();
    std::cout << "Stemmed: " << stemmer.toString() << std::endl;
    std::cout << "Stemmed: " << stemmer.getResultLength() << std::endl;
    display(stemmer.toString(),stemmer.getResultLength());
    return 0;
}