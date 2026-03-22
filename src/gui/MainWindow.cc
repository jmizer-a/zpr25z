#include "MainWindow.h"
#include <QHBoxLayout>
#include <QInputDialog>
#include "core/Property.h"
#include "core/SpecialProperties.h"

using namespace std;

MainWindow::MainWindow( QWidget* parent ) : QWidget( parent ) {
    setWindowTitle( "Elkopoly" );
    resize( 1600, 1080 );
    QHBoxLayout* rootLayout = new QHBoxLayout( this );

    QScrollArea* playerListScroll = new QScrollArea( this );
    QWidget* playerListWidget = new QWidget();
    playerListLayout_ = new QVBoxLayout( playerListWidget );
    playerListLayout_->setAlignment( Qt::AlignTop );
    playerListScroll->setWidget( playerListWidget );
    playerListScroll->setWidgetResizable( true );
    playerListScroll->setFixedWidth( 220 );
    rootLayout->addWidget( playerListScroll, 1 );

    QVBoxLayout* centerLayout = new QVBoxLayout();

    QHBoxLayout* topBar = new QHBoxLayout();
    currentPlayerLabel_ = new QLabel( "Oczekiwanie...", this );
    currentPlayerLabel_->setStyleSheet( "font-weight: bold; font-size: 20px;" );

    cashLabel_ = new QLabel( "$0", this );
    cashLabel_->setStyleSheet( "font-weight: bold; font-size: 20px; color: #27AE60;" );

    topBar->addWidget( currentPlayerLabel_ );
    topBar->addStretch();
    topBar->addWidget( cashLabel_ );
    centerLayout->addLayout( topBar );

    boardLayout_ = new QGridLayout();
    boardLayout_->setSpacing( 2 );
    setupBoardGrid();
    centerLayout->addLayout( boardLayout_ );

    rollButton_ = new QPushButton( "RZUĆ KOŚĆMI", this );
    rollButton_->setMinimumHeight( 60 );
    rollButton_->setStyleSheet( "font-weight: bold; font-size: 16px;" );
    rollButton_->setEnabled( false );
    centerLayout->addWidget( rollButton_ );

    rootLayout->addLayout( centerLayout, 3 );

    QScrollArea* scroll = new QScrollArea( this );
    cardsScrollWidget_ = new QWidget();
    cardsLayout_ = new QVBoxLayout( cardsScrollWidget_ );
    cardsLayout_->setAlignment( Qt::AlignTop );

    scroll->setWidget( cardsScrollWidget_ );
    scroll->setWidgetResizable( true );
    scroll->setFixedWidth( 260 );

    rootLayout->addWidget( scroll, 1 );

    this->setStyleSheet( "QWidget { background-color: #F0F0F0; color: black; }"
                         "QPushButton { background-color: #E1E1E1; color: black; border: 1px solid #ADADAD; }"
                         "QLineEdit { background-color: white; color: black; }"
                         "QScrollArea { background-color: #F0F0F0; }" );
}

void MainWindow::setupBoardGrid() {
    spaceLabels_.resize( 40 );
    for( int i = 0; i < 40; ++i ) {
        QLabel* label = new QLabel( this );
        label->setFrameStyle( QFrame::Box | QFrame::Plain );
        label->setFixedSize( 85, 75 );
        label->setWordWrap( true );

        int row = 0, col = 0;
        if( i <= 10 ) {
            row = 0;
            col = i;
        } else if( i <= 20 ) {
            row = i - 10;
            col = 10;
        } else if( i <= 30 ) {
            row = 10;
            col = 10 - ( i - 20 );
        } else {
            row = 10 - ( i - 30 );
            col = 0;
        }

        boardLayout_->addWidget( label, row, col );
        spaceLabels_[i] = label;
    }

    QLabel* bigLabel = new QLabel( "ELKOPOLY", this );
    bigLabel->setAlignment( Qt::AlignCenter );
    bigLabel->setStyleSheet(
        "font-family: 'Arial Black'; font-size: 90px; font-weight: bold; color: #C0392B; letter-spacing: 12px;" );
    boardLayout_->addWidget( bigLabel, 1, 1, 9, 9 );
}

QString MainWindow::getHexColor( Color color ) {
    switch( color ) {
        case Color::BROWN:
            return "#955436";
        case Color::LIGHTBLUE:
            return "#AAE0FA";
        case Color::PINK:
            return "#D93A96";
        case Color::ORANGE:
            return "#F7941D";
        case Color::RED:
            return "#ED1B24";
        case Color::YELLOW:
            return "#FEF200";
        case Color::GREEN:
            return "#1FB25A";
        case Color::DARKBLUE:
            return "#0072BB";
        default:
            return "#FFFFFF";
    }
}

void MainWindow::displayBoard( const vector<unique_ptr<Space>>& board ) {
    for( size_t i = 0; i < spaceLabels_.size() && i < board.size(); ++i ) {
        Space* space = board[i].get();
        spaceLabels_[i]->setText( QString::fromStdString( space->getName() ) );

        Card* card = space->getAssociatedCard();
        QString bgColor = "#FFFFFF";

        if( card ) {
            if( Property* prop = dynamic_cast<Property*>( card ) ) {
                bgColor = getHexColor( prop->getColor() );
            } else if( dynamic_cast<Railway*>( card ) ) {
                bgColor = "#fde9e9";
            } else if( dynamic_cast<Utility*>( card ) ) {
                bgColor = "#d7fdd7";
            }
        }

        spaceLabels_[i]->setStyleSheet(
            QString( "background-color: %1; font-size: 9px; border: 2px solid black; font-weight: bold; "
                     "qproperty-alignment: 'AlignHCenter | AlignTop'; padding-top: 2px;" )
                .arg( bgColor ) );
    }
}

void MainWindow::updatePlayerStats( const QString& name, unsigned int cash ) {
    currentPlayerLabel_->setText( QString( "Gracz: %1" ).arg( name ) );
    cashLabel_->setText( QString( "Portfel: $%1" ).arg( cash ) );
}

void MainWindow::refreshPlayerData( Player* player ) {
    if( !player )
        return;
    updatePlayerStats( QString::fromStdString( player->getName() ), player->getCash() );
    updateOwnedCards( player->getOwnedCards() );
}

void MainWindow::updateOwnedCards( const vector<Card*>& cards ) {
    QLayoutItem* child;
    while( ( child = cardsLayout_->takeAt( 0 ) ) != nullptr ) {
        if( child->widget() ) {
            child->widget()->deleteLater();
        }
        delete child;
    }

    for( size_t i = 0; i < cards.size(); ++i ) {
        Card* card = cards[i];
        if( !card )
            continue;

        QWidget* container = new QWidget();
        QVBoxLayout* layout = new QVBoxLayout( container );

        QLabel* label = new QLabel( QString::fromStdString( card->describe() ) );
        label->setWordWrap( true );
        label->setAlignment( Qt::AlignCenter );
        label->setMinimumHeight( 250 );

        QString bgColor = "#FFFFFF";
        Property* prop = dynamic_cast<Property*>( card );
        if( prop )
            bgColor = getHexColor( prop->getColor() );

        label->setStyleSheet(
            QString( "background-color: %1; border: 1px solid black; padding: 5px;" ).arg( bgColor ) );
        layout->addWidget( label );

        if( PurchasableCard* pc = dynamic_cast<PurchasableCard*>( card ) ) {
            QHBoxLayout* row = new QHBoxLayout();
            if( prop ) {
                QPushButton* b = new QPushButton( "Buduj" );
                connect( b, &QPushButton::clicked, [this, i]() {
                    if( onBuildClicked_ )
                        onBuildClicked_( i );
                } );
                row->addWidget( b );
            }
            QPushButton* m = new QPushButton( pc->isMortgaged() ? "Wykup" : "Zastaw" );
            connect( m, &QPushButton::clicked, [this, i]() {
                if( onMortgageClicked_ )
                    onMortgageClicked_( i );
            } );
            row->addWidget( m );
            layout->addLayout( row );
        }
        cardsLayout_->addWidget( container );
    }
}

void MainWindow::waitForRoll() {
    rollButton_->setEnabled( true );
    QEventLoop loop;
    connect( rollButton_, &QPushButton::clicked, &loop, &QEventLoop::quit );
    loop.exec();
    rollButton_->setEnabled( false );
}

void MainWindow::updatePlayerPositions( const unordered_map<Player*, unsigned int>& positions ) {
    vector<Player*> sortedPlayers;
    for( auto const& [player, pos] : positions ) {
        if( player )
            sortedPlayers.push_back( player );
    }
    sort( sortedPlayers.begin(), sortedPlayers.end(), []( Player* a, Player* b ) {
        return a->getName() < b->getName();
    } );

    for( Player* player : sortedPlayers ) {
        unsigned int position = positions.at( player );

        if( playerPawns_.find( player ) == playerPawns_.end() ) {
            QLabel* pawn = new QLabel( this );
            pawn->setText( QString::fromStdString( player->getName() ) );
            pawn->setStyleSheet(
                "font-style: italic; font-weight: bold; font-size: 10px; border: 1px solid black; padding: 1px;" );
            pawn->setAttribute( Qt::WA_TransparentForMouseEvents );
            playerPawns_[player] = pawn;

            QWidget* sectionWidget = new QWidget();
            QVBoxLayout* sectionLayout = new QVBoxLayout( sectionWidget );
            sectionWidget->setStyleSheet( "border-bottom: 1px solid gray; padding: 5px;" );
            playerListLayout_->addWidget( sectionWidget );
            playerSectionMap_[player] = sectionLayout;
        }

        QVBoxLayout* section = playerSectionMap_[player];
        QLayoutItem* item;
        while( ( item = section->takeAt( 0 ) ) != nullptr ) {
            if( item->widget() )
                item->widget()->deleteLater();
            delete item;
        }

        QLabel* nameLabel = new QLabel( QString::fromStdString( player->getName() ) );
        nameLabel->setStyleSheet( "font-weight: bold; font-size: 14px; color: black;" );
        section->addWidget( nameLabel );

        QLabel* pCashLabel = new QLabel( QString( "Portfel: $%1" ).arg( player->getCash() ) );
        pCashLabel->setStyleSheet( "color: #27AE60; font-weight: bold;" );
        section->addWidget( pCashLabel );

        for( Card* card : player->getOwnedCards() ) {
            QString cardText = QString::fromStdString( " • " + card->getName() + "; " );
            if( Property* prop = dynamic_cast<Property*>( card ) ) {
                if( prop->getHotelCount() > 0 )
                    cardText += QString( " [Hotele: %1]" ).arg( prop->getHotelCount() );
                else if( prop->getHouseCount() > 0 )
                    cardText += QString( " [Domy: %1]" ).arg( prop->getHouseCount() );
            }
            QLabel* cardLabel = new QLabel( cardText );
            cardLabel->setStyleSheet( "font-size: 11px; color: #555;" );
            section->addWidget( cardLabel );
        }

        QLabel* pawn = playerPawns_[player];
        if( position < spaceLabels_.size() ) {
            QLabel* targetSpace = spaceLabels_[position];
            pawn->setParent( targetSpace );
            int offset = 0;
            for( Player* otherPlayer : sortedPlayers ) {
                if( otherPlayer == player )
                    break;
                auto it = positions.find( otherPlayer );
                if( it != positions.end() && it->second == position )
                    offset += 15;
            }
            pawn->move( 5, 45 + offset );
            pawn->show();
            pawn->raise();
        }
    }
}

PlayerDecision MainWindow::askBuyOrAuction( const string& name, unsigned int price, unsigned int cash ) {
    QString info = QString( "Stoisz na: %1\nCena: $%2\n\nCzy chcesz kupić tę nieruchomość?" )
                       .arg( QString::fromStdString( name ) )
                       .arg( price );
    QMessageBox::StandardButton reply =
        QMessageBox::question( this, "Zakup nieruchomości", info, QMessageBox::Yes | QMessageBox::No );
    return ( reply == QMessageBox::Yes ) ? PlayerDecision::BUY : PlayerDecision::AUCTION;
}

void MainWindow::showRandomCard( const string& description ) {
    QMessageBox::information( this, "Karta Szansy / Kasa Społeczna", QString::fromStdString( description ) );
}

unsigned int MainWindow::askBid( const string& name, unsigned int current_bid, unsigned int cash ) {
    bool ok;
    QString prompt = QString( "Licytacja: %1\nObecna oferta: $%2\nTwoja gotówka: $%3\n\nPodaj kwotę (0 aby spasować):" )
                         .arg( QString::fromStdString( name ) )
                         .arg( current_bid )
                         .arg( cash );
    int bid = QInputDialog::getInt( this, "Aukcja", prompt, 0, 0, cash, 1, &ok );
    return ( ok && static_cast<unsigned int>( bid ) > current_bid ) ? static_cast<unsigned int>( bid ) : 0;
}

bool MainWindow::askUseJailCard() {
    QMessageBox::StandardButton reply =
        QMessageBox::question( this,
                               "Więzienie",
                               "Masz kartę 'Ucieczka z więzienia'. Czy chcesz jej użyć?",
                               QMessageBox::Yes | QMessageBox::No );
    return ( reply == QMessageBox::Yes );
}

bool MainWindow::askWantsToTrade( const string& player_name ) {
    QMessageBox::StandardButton reply =
        QMessageBox::question( this,
                               "Handel",
                               QString::fromStdString( player_name ) + ", czy chcesz zaproponować transakcję?",
                               QMessageBox::Yes | QMessageBox::No );
    return ( reply == QMessageBox::Yes );
}

TradeOffer MainWindow::createTradeOffer( Player* requester, const vector<unique_ptr<Player>>& all_players ) {
    TradeOffer offer;
    if( !requester )
        return offer;

    QStringList playerNames;
    vector<Player*> targets;
    for( const auto& p : all_players ) {
        if( p.get() != requester && !p->isLost() ) {
            playerNames << QString::fromStdString( p->getName() );
            targets.push_back( p.get() );
        }
    }

    if( targets.empty() )
        return offer;

    bool ok;
    QString targetName = QInputDialog::getItem( this, "Handel", "Wybierz gracza:", playerNames, 0, false, &ok );
    int targetIdx = playerNames.indexOf( targetName );
    if( !ok || targetIdx < 0 )
        return offer;

    offer.receiver_ = targets[targetIdx];
    offer.cashOffered_ =
        QInputDialog::getInt( this, "Handel", "Zaoferowana gotówka:", 0, 0, requester->getCash(), 10, &ok );
    offer.cashRequested_ =
        QInputDialog::getInt( this, "Handel", "Żądana gotówka:", 0, 0, offer.receiver_->getCash(), 10, &ok );

    QStringList myCards;
    myCards << "Brak";
    for( auto* c : requester->getOwnedCards() ) {
        if( dynamic_cast<PurchasableCard*>( c ) )
            myCards << QString::fromStdString( c->getName() );
    }

    QString cardOfferedName =
        QInputDialog::getItem( this, "Handel", "Twoja karta na wymianę:", myCards, 0, false, &ok );
    if( ok && cardOfferedName != "Brak" ) {
        for( auto* c : requester->getOwnedCards() ) {
            if( c->getName() == cardOfferedName.toStdString() ) {
                offer.cardOffered_ = static_cast<PurchasableCard*>( c );
                break;
            }
        }
    }

    QStringList theirCards;
    theirCards << "Brak";
    for( auto* c : offer.receiver_->getOwnedCards() ) {
        if( dynamic_cast<PurchasableCard*>( c ) )
            theirCards << QString::fromStdString( c->getName() );
    }

    QString cardRequestedName =
        QInputDialog::getItem( this, "Handel", "Karta, którą chcesz otrzymać:", theirCards, 0, false, &ok );
    if( ok && cardRequestedName != "Brak" ) {
        for( auto* c : offer.receiver_->getOwnedCards() ) {
            if( c->getName() == cardRequestedName.toStdString() ) {
                offer.cardRequested_ = static_cast<PurchasableCard*>( c );
                break;
            }
        }
    }

    offer.requester_ = requester;

    return offer;
}

bool MainWindow::askAcceptTrade( Player* receiver, TradeOffer offer ) {
    if( !receiver )
        return false;

    QString details =
        QString( "%1, otrzymałaś/łeś ofertę od gracza %2:" )
            .arg( QString::fromStdString( receiver->getName() ) )
            .arg( offer.requester_ ? QString::fromStdString( offer.requester_->getName() ) : "Nieznany gracz" );
    details += QString( "Dajesz: $%1, %2\n" )
                   .arg( offer.cashRequested_ )
                   .arg( offer.cardRequested_ ? QString::fromStdString( offer.cardRequested_->getName() ) : "" );
    details += QString( "Otrzymujesz: $%1, %2\n\n" )
                   .arg( offer.cashOffered_ )
                   .arg( offer.cardOffered_ ? QString::fromStdString( offer.cardOffered_->getName() ) : "" );
    details += "Czy akceptujesz tę ofertę?";

    QMessageBox::StandardButton reply =
        QMessageBox::question( this, "Propozycja handlu", details, QMessageBox::Yes | QMessageBox::No );
    return ( reply == QMessageBox::Yes );
}

void MainWindow::displayWinner( Player* player ) {
    if( player ) {
        QString message =
            QString( "Koniec gry! Zwycięzcą jest: %1" ).arg( QString::fromStdString( player->getName() ) );
        QMessageBox msgBox( this );
        msgBox.setWindowTitle( "Koniec gry" );
        msgBox.setText( message );
        msgBox.setIcon( QMessageBox::Information );
        msgBox.exec();
    }
}
